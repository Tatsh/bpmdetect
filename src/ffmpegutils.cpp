#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTemporaryFile>
extern "C" {
#include <libavformat/avformat.h>
}

#include "debug.h"
#include "ffmpegutils.h"

// https://github.com/joncampbell123/composite-video-simulator/issues/5#issuecomment-611885908
#ifdef av_err2str
#undef av_err2str
#include <string>
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif // av_err2str

bool isDecodableFile(const QString &fileName) {
    AVFormatContext *fmt_ctx = nullptr;
    int ret;
    if ((ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr)) !=
        0) {
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_open_input() returned" << ret << av_err2str(ret);
        return false;
    }
    if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_find_stream_info() returned" << ret
                                  << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    auto hasAudio = false;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            hasAudio = true;
            break;
        }
    }
    avformat_close_input(&fmt_ctx);
    qCDebug(gLogBpmDetect) << "File:" << fileName << "has audio:" << hasAudio;
    return hasAudio;
}

static QString getTemporaryFileName(const QString &fileName) {
    QTemporaryFile tempFile;
    tempFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/XXXXXX.") +
                             QFileInfo(fileName).suffix());
    tempFile.open();
    tempFile.close();
    return tempFile.fileName();
}

bool storeBpmInFile(const QString &fileName, const QString &sBpm) {
    qCDebug(gLogBpmDetect) << "Storing BPM:" << sBpm << "to file:" << fileName;
    AVFormatContext *fmt_ctx = nullptr;
    auto ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_open_input() returned" << ret << av_err2str(ret);
        return false;
    }
    // Retrieve stream information.
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_find_stream_info() returned" << ret
                                  << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    // Set BPM metadata.
    const auto key =
        fmt_ctx->iformat &&
                QString::fromUtf8(fmt_ctx->iformat->name).contains(QStringLiteral("mp3")) ?
            "TBPM" :
            "bpm";
    av_dict_set(&fmt_ctx->metadata, key, sBpm.toUtf8().constData(), 0);
    // Prepare output file name.
    auto outFile = getTemporaryFileName(fileName);
    qCDebug(gLogBpmDetect) << "Temporary file for updated metadata:" << outFile;
    AVFormatContext *out_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outFile.toUtf8().constData());
    if (ret < 0 || !out_ctx) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to allocate output context for file:"
                                  << outFile << ". avformat_alloc_output_context2() returned" << ret
                                  << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    // Copy streams from input to output.
    for (auto i = 0; i < fmt_ctx->nb_streams; ++i) {
        auto in_stream = fmt_ctx->streams[i];
        auto *out_stream = avformat_new_stream(out_ctx, nullptr);
        if (!out_stream) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect) << "libavformat failed to create output stream.";
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect)
                << "libavformat failed to copy codec parameters. avcodec_parameters_copy() returned"
                << ret << av_err2str(ret);
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
        out_stream->time_base = in_stream->time_base;
    }
    // Copy global metadata.
    av_dict_copy(&out_ctx->metadata, fmt_ctx->metadata, 0);
    // Open output file.
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, outFile.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect) << "libavformat failed to open output file:" << outFile
                                      << ". avio_open() returned" << ret << av_err2str(ret);
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
    }
    // Write header.
    ret = avformat_write_header(out_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to write header to output file:" << outFile
                                  << ". avformat_write_header() returned" << ret << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_ctx->pb);
        }
        avformat_free_context(out_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    // Write packets (copy mode).
    AVPacket pkt;
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        // Find output stream index.
        pkt.stream_index = pkt.stream_index;
        ret = av_interleaved_write_frame(out_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect)
                << "libavformat failed to write frame. av_interleaved_write_frame() returned" << ret
                << av_err2str(ret);
            break;
            // LCOV_EXCL_STOP
        }
    }
    // Write trailer.
    av_write_trailer(out_ctx);
    // Clean up.
    avformat_close_input(&fmt_ctx);
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&out_ctx->pb);
    }
    avformat_free_context(out_ctx);
    // Replace original file with new file.
    if (!QFile::remove(fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to delete original file" << fileName;
        return false;
        // LCOV_EXCL_STOP
    }
    if (!QFile::rename(outFile, fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to replace original file with updated metadata file:"
                                  << fileName;
        return false;
        // LCOV_EXCL_STOP
    }
    return true;
}

bool removeBpmFromFile(const QString &fileName) {
    qCDebug(gLogBpmDetect) << "Removing BPM metadata from file:" << fileName;
    AVFormatContext *fmt_ctx = nullptr;
    auto ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_open_input() returned" << ret << av_err2str(ret);
        return false;
    }
    // Retrieve stream information.
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_find_stream_info() returned" << ret
                                  << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    // Remove BPM from global metadata.
    const auto key =
        fmt_ctx->iformat &&
                QString::fromUtf8(fmt_ctx->iformat->name).contains(QStringLiteral("mp3")) ?
            "TBPM" :
            "bpm";
    av_dict_set(&fmt_ctx->metadata, key, nullptr, 0);
    // Prepare output file name.
    auto outFile = getTemporaryFileName(fileName);
    // Open output context.
    AVFormatContext *out_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outFile.toUtf8().constData());
    if (ret < 0 || !out_ctx) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to allocate output context for file:"
                                  << outFile << ". avformat_alloc_output_context2() returned" << ret
                                  << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    // Copy streams from input to output.
    for (auto i = 0; i < fmt_ctx->nb_streams; ++i) {
        auto in_stream = fmt_ctx->streams[i];
        auto out_stream = avformat_new_stream(out_ctx, nullptr);
        if (!out_stream) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect) << "libavformat failed to create output stream.";
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect) << "libavformat failed to copy codec parameters.";
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
        out_stream->time_base = in_stream->time_base;
        // Copy stream metadata except TBPM for MP3.
        auto bpmKey =
            fmt_ctx->iformat &&
                    QString::fromUtf8(fmt_ctx->iformat->name).contains(QStringLiteral("mp3")) ?
                QStringLiteral("TBPM") :
                QStringLiteral("bpm");
        AVDictionary *newTags = nullptr;
        AVDictionaryEntry *entry = nullptr;
        while ((entry = av_dict_get(in_stream->metadata, "", entry, AV_DICT_IGNORE_SUFFIX))) {
            if (QString::fromUtf8(entry->key) != bpmKey) {
                av_dict_set(&newTags, entry->key, entry->value, 0);
            }
        }
        out_stream->metadata = newTags;
    }
    // Copy global metadata (BPM already removed).
    av_dict_copy(&out_ctx->metadata, fmt_ctx->metadata, 0);
    // Open output file.
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, outFile.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect) << "libavformat failed to open output file:" << outFile
                                      << ". avio_open() returned" << ret << av_err2str(ret);
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
    }
    // Write header.
    ret = avformat_write_header(out_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to write header to output file:" << outFile
                                  << ". avformat_write_header() returned" << ret << av_err2str(ret);
        avformat_close_input(&fmt_ctx);
        if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_ctx->pb);
        }
        avformat_free_context(out_ctx);
        return false;
        // LCOV_EXCL_STOP
    }
    // Write packets (copy mode).
    AVPacket pkt;
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        ret = av_interleaved_write_frame(out_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect)
                << "libavformat failed to write frame. av_interleaved_write_frame() returned" << ret
                << av_err2str(ret);
            break;
            // LCOV_EXCL_STOP
        }
    }
    // Write trailer.
    av_write_trailer(out_ctx);
    // Cleanup
    avformat_close_input(&fmt_ctx);
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&out_ctx->pb);
    }
    avformat_free_context(out_ctx);
    // Replace original file with new file.
    if (!QFile::remove(fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to delete original file" << fileName;
        return false;
        // LCOV_EXCL_STOP
    }
    if (!QFile::rename(outFile, fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to replace original file with updated metadata file:"
                                  << fileName;
        return false;
        // LCOV_EXCL_STOP
    }
    return true;
}

static void readTag(const AVFormatContext *fmt_ctx,
                    const AVDictionary *dict,
                    const QString &sourceKey,
                    const QString &targetKey,
                    QMap<QString, QVariant> &tags) {
    auto sourceKeyData = sourceKey.toUtf8().constData();
    auto entry = av_dict_get(dict, sourceKeyData, nullptr, 0);
    auto val = entry ? QString::fromUtf8(entry->value).trimmed() : QString();
    if (!val.isEmpty()) {
        tags[targetKey] = val;
    } else {
        for (auto i = 0; i < fmt_ctx->nb_streams; ++i) {
            auto stream = fmt_ctx->streams[i];
            auto entry = av_dict_get(stream->metadata, sourceKeyData, nullptr, 0);
            auto val = entry ? QString::fromUtf8(entry->value).trimmed() : QString();
            if (!val.isEmpty()) {
                tags[targetKey] = val;
                break;
            }
        }
    }
}

QMap<QString, QVariant> readTagsFromFile(const QString &fileName) {
    AVFormatContext *fmt_ctx = nullptr;
    QMap<QString, QVariant> returnTags;
    returnTags.insert(QStringLiteral("artist"), QVariant(QStringLiteral("")));
    returnTags.insert(QStringLiteral("title"), QVariant(QStringLiteral("")));
    returnTags.insert(QStringLiteral("bpm"), QVariant(0));
    returnTags.insert(QStringLiteral("length"), QVariant(0));
    int ret;
    if ((ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr)) ==
        0) {
        if (avformat_find_stream_info(fmt_ctx, nullptr) >= 0) {
            readTag(fmt_ctx,
                    fmt_ctx->metadata,
                    QStringLiteral("artist"),
                    QStringLiteral("artist"),
                    returnTags);
            readTag(fmt_ctx,
                    fmt_ctx->metadata,
                    QStringLiteral("title"),
                    QStringLiteral("title"),
                    returnTags);
            const auto key =
                fmt_ctx->iformat &&
                        QString::fromUtf8(fmt_ctx->iformat->name).contains(QStringLiteral("mp3")) ?
                    QStringLiteral("TBPM") :
                    QStringLiteral("bpm");
            readTag(fmt_ctx, fmt_ctx->metadata, key, QStringLiteral("bpm2"), returnTags);
            // Convert BPM to double.
            auto ok = false;
            auto bpmVal = returnTags[QStringLiteral("bpm2")].toString().toDouble(&ok);
            if (ok) {
                returnTags[QStringLiteral("bpm")] = bpmVal;
            } else {
                returnTags[QStringLiteral("bpm")] = 0;
            }
            returnTags.remove(QStringLiteral("bpm2"));
            // Read length in milliseconds.
            if (fmt_ctx->duration != AV_NOPTS_VALUE) {
                returnTags[QStringLiteral("length")] =
                    static_cast<qint64>(fmt_ctx->duration / (AV_TIME_BASE / 1000));
            }
        }
        avformat_close_input(&fmt_ctx);
    } else {
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_open_input() returned" << ret << av_err2str(ret);
    }
    return returnTags;
}
