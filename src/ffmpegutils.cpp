#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
extern "C" {
#include <libavformat/avformat.h>
}

#include "debug.h"
#include "ffmpegutils.h"

bool isDecodableFile(const QString &file) {
    AVFormatContext *fmt_ctx = nullptr;
    auto url = QStringLiteral("file://") + file;
    if (avformat_open_input(&fmt_ctx, url.toUtf8().constData(), nullptr, nullptr) != 0) {
        return false;
    }
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        // LCOV_EXCL_START
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
    return hasAudio;
}

static QString makeTempFileName(const QString &fileName) {
    QFileInfo info(fileName);
    return info.absolutePath() + QStringLiteral("/.~") + info.fileName();
}

bool storeBpmInFile(const QString &fileName, const QString &sBpm) {
    qCDebug(gLogBpmDetect) << "Storing BPM:" << sBpm << "to file:" << fileName;
    AVFormatContext *fmt_ctx = nullptr;
    auto ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName;
        return false;
    }
    // Retrieve stream information.
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to find stream info for file:" << fileName;
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
    auto outFile = makeTempFileName(fileName);
    AVFormatContext *out_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outFile.toUtf8().constData());
    if (ret < 0 || !out_ctx) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to allocate output context for file:"
                                  << outFile;
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
            qCCritical(gLogBpmDetect) << "libavformat failed to copy codec parameters.";
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
            qCCritical(gLogBpmDetect) << "libavformat failed to open output file:" << outFile;
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
        qCCritical(gLogBpmDetect) << "libavformat failed to write header to output file:"
                                  << outFile;
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
            qCCritical(gLogBpmDetect) << "libavformat failed to write frame.";
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
    QFile::remove(fileName);
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
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName;
        return false;
    }
    // Retrieve stream information.
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to find stream info for file:" << fileName;
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
    auto outFile = makeTempFileName(fileName);
    // Open output context.
    AVFormatContext *out_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outFile.toUtf8().constData());
    if (ret < 0 || !out_ctx) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "libavformat failed to allocate output context for file:"
                                  << outFile;
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
            qCCritical(gLogBpmDetect) << "libavformat failed to open output file:" << outFile;
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
        qCCritical(gLogBpmDetect) << "libavformat failed to write header to output file:"
                                  << outFile;
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
            qCCritical(gLogBpmDetect) << "libavformat failed to write frame.";
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
    QFile::remove(fileName);
    if (!QFile::rename(outFile, fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to replace original file with updated metadata file:"
                                  << fileName;
        return false;
        // LCOV_EXCL_STOP
    }
    return true;
}

QMap<QString, QVariant> readTagsFromFile(const QString &fileName) {
    AVFormatContext *fmt_ctx = nullptr;
    QMap<QString, QVariant> returnTags;
    returnTags.insert(QStringLiteral("artist"), QVariant(QStringLiteral("")));
    returnTags.insert(QStringLiteral("title"), QVariant(QStringLiteral("")));
    returnTags.insert(QStringLiteral("bpm"), QVariant(0));
    returnTags.insert(QStringLiteral("length"), QVariant(0));
    if (avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr) == 0) {
        if (avformat_find_stream_info(fmt_ctx, nullptr) >= 0) {
            auto artistEntry = av_dict_get(fmt_ctx->metadata, "artist", nullptr, 0);
            if (artistEntry && !QString::fromUtf8(artistEntry->value).isEmpty()) {
                returnTags[QStringLiteral("artist")] = QString::fromUtf8(artistEntry->value);
            }
            auto titleEntry = av_dict_get(fmt_ctx->metadata, "title", nullptr, 0);
            if (titleEntry && !QString::fromUtf8(titleEntry->value).isEmpty()) {
                returnTags[QStringLiteral("title")] = QString::fromUtf8(titleEntry->value);
            }
            auto key =
                fmt_ctx->iformat &&
                        QString::fromUtf8(fmt_ctx->iformat->name).contains(QStringLiteral("mp3")) ?
                    "TBPM" :
                    "bpm";
            auto bpmEntry = av_dict_get(fmt_ctx->metadata, key, nullptr, 0);
            if (bpmEntry && !QString::fromUtf8(bpmEntry->value).isEmpty()) {
                auto ok = false;
                auto bpmVal = QString::fromUtf8(bpmEntry->value).toDouble(&ok);
                if (ok) {
                    returnTags[QStringLiteral("bpm")] = bpmVal;
                }
            }
            if (fmt_ctx->duration != AV_NOPTS_VALUE) {
                returnTags[QStringLiteral("length")] =
                    static_cast<qint64>(fmt_ctx->duration / (AV_TIME_BASE / 1000));
            }
        }
        avformat_close_input(&fmt_ctx);
    }
    return returnTags;
}
