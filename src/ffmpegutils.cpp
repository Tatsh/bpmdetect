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
#include "utils.h"

static const auto kBpmKeyTBpm = QStringLiteral("TBPM");
static const auto kBpmKeyTmpo = QStringLiteral("tmpo");
static const auto kBpmKeyBpm = QStringLiteral("bpm");
static const auto kNameM4a = QStringLiteral("m4a");
static const auto kNameMp3 = QStringLiteral("mp3");

// https://github.com/joncampbell123/composite-video-simulator/issues/5#issuecomment-611885908
Q_ALWAYS_INLINE QString av_errToQString(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return QString::fromUtf8(av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum));
}

static QString lastError;

static void setLastError(const QString &error) {
    lastError = error;
}

QString getLastError() {
    return lastError;
}

bool isDecodableFile(const QString &fileName) {
    AVFormatContext *fmt_ctx = nullptr;
    int ret;
    if ((ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr)) !=
        0) {
        auto errStr = av_errToQString(ret);
        // LCOV_EXCL_START
        qCDebug(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                               << ". avformat_open_input() returned" << ret << errStr;
        // LCOV_EXCL_STOP
        setLastError(errStr);
        return false;
    }
    if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) < 0) {
        // LCOV_EXCL_START
        auto errStr = av_errToQString(ret);
        qCDebug(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                               << ". avformat_find_stream_info() returned" << ret << errStr;
        setLastError(errStr);
        avformat_close_input(&fmt_ctx);
        return false;
    }
    // LCOV_EXCL_STOP
    auto hasAudio = false;
    for (AVStream *stream : unsafeSpan(fmt_ctx->streams, fmt_ctx->nb_streams)) {
        if (stream && stream->codecpar && stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            hasAudio = true;
            break;
        }
    }
    avformat_close_input(&fmt_ctx);
    qCDebug(gLogBpmDetect) << "File:" << fileName << "has audio:" << hasAudio;
    setLastError(QString());
    return hasAudio;
}

static QString getTemporaryFileName(const QString &fileName, bool *error) {
    QTemporaryFile tempFile;
    static const auto tempFileTemplate = QStringLiteral("/XXXXXX.");
    tempFile.setFileTemplate(QDir::tempPath() + tempFileTemplate + QFileInfo(fileName).suffix());
    *error = !tempFile.open();
    if (*error) {
        // LCOV_EXCL_START
        qCDebug(gLogBpmDetect) << "Failed to create temporary file." << tempFile.errorString();
        setLastError(tempFile.errorString());
        return QString();
        // LCOV_EXCL_STOP
    }
    tempFile.close();
    setLastError(QString());
    return tempFile.fileName();
}

bool storeBpmInFile(const QString &fileName, const QString &sBpm) {
    qCDebug(gLogBpmDetect) << "Storing BPM:" << sBpm << "to file:" << fileName;
    AVFormatContext *fmt_ctx = nullptr;
    auto ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        auto errStr = av_errToQString(ret);
        // LCOV_EXCL_START
        qCDebug(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                               << ". avformat_open_input() returned" << ret << errStr;
        // LCOV_EXCL_STOP
        setLastError(errStr);
        return false;
    }
    // Retrieve stream information.
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        auto errStr = av_errToQString(ret);
        qCDebug(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                               << ". avformat_find_stream_info() returned" << ret << errStr;
        avformat_close_input(&fmt_ctx);
        setLastError(errStr);
        return false;
        // LCOV_EXCL_STOP
    }
    // Set BPM metadata.
    const auto name = QString::fromUtf8(fmt_ctx->iformat ? fmt_ctx->iformat->name : "");
    const auto key = name.contains(kNameMp3) ? "TBPM" : name.contains(kNameM4a) ? "tmpo" : "bpm";
    qCDebug(gLogBpmDetect) << "Using metadata key:" << key;
    av_dict_set(&fmt_ctx->metadata, key, sBpm.toUtf8().constData(), 0);
    // Prepare output file name.
    bool error = false;
    auto outFile = getTemporaryFileName(fileName, &error);
    if (error) {
        avformat_close_input(&fmt_ctx);
        return false;
    }
    qCDebug(gLogBpmDetect) << "Temporary file for updated metadata:" << outFile;
    AVFormatContext *out_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outFile.toUtf8().constData());
    if (ret < 0 || !out_ctx) {
        // LCOV_EXCL_START
        auto errStr = av_errToQString(ret);
        qCDebug(gLogBpmDetect) << "libavformat failed to allocate output context for file:"
                               << outFile << ". avformat_alloc_output_context2() returned" << ret
                               << errStr;
        avformat_close_input(&fmt_ctx);
        setLastError(errStr);
        return false;
    }
    // LCOV_EXCL_STOP
    // Copy streams from input to output.
    auto streams = unsafeSpan(fmt_ctx->streams, fmt_ctx->nb_streams);
    for (const auto *in_stream : streams) {
        auto *out_stream = avformat_new_stream(out_ctx, nullptr);
        if (!out_stream) {
            // LCOV_EXCL_START
            qCDebug(gLogBpmDetect) << "libavformat failed to create output stream.";
            setLastError(QObject::tr("libavformat failed to create output stream."));
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            // LCOV_EXCL_START
            auto errStr = av_errToQString(ret);
            qCDebug(gLogBpmDetect)
                << "libavformat failed to copy codec parameters. avcodec_parameters_copy() returned"
                << ret << errStr;
            setLastError(errStr);
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
        }
        // LCOV_EXCL_STOP
        out_stream->time_base = in_stream->time_base;
    }
    // Copy global metadata.
    av_dict_copy(&out_ctx->metadata, fmt_ctx->metadata, 0);
    // Open output file.
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, outFile.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            // LCOV_EXCL_START
            auto errStr = av_errToQString(ret);
            qCDebug(gLogBpmDetect) << "libavformat failed to open output file:" << outFile
                                   << ". avio_open() returned" << ret << errStr;
            setLastError(errStr);
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
        }
        // LCOV_EXCL_STOP
    }
    // Write header.
    ret = avformat_write_header(out_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        auto errStr = av_errToQString(ret);
        qCDebug(gLogBpmDetect) << "libavformat failed to write header to output file:" << outFile
                               << ". avformat_write_header() returned" << ret << errStr;
        setLastError(errStr);
        avformat_close_input(&fmt_ctx);
        if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_ctx->pb);
        }
        avformat_free_context(out_ctx);
        return false;
    }
    // LCOV_EXCL_STOP
    // Write packets (copy mode).
    AVPacket pkt;
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        // Find output stream index.
        pkt.stream_index = pkt.stream_index;
        ret = av_interleaved_write_frame(out_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            // LCOV_EXCL_START
            auto errStr = av_errToQString(ret);
            qCCritical(gLogBpmDetect)
                << "libavformat failed to write frame. av_interleaved_write_frame() returned" << ret
                << errStr;
            setLastError(errStr);
            break;
        }
        // LCOV_EXCL_STOP
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
    QFile fi(fileName);
    if (!fi.remove(fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to delete original file" << fileName << "."
                                  << fi.errorString();
        setLastError(fi.errorString());
        return false;
        // LCOV_EXCL_STOP
    }
    if (!fi.rename(outFile, fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to replace original file with updated metadata file:"
                                  << fileName << "." << fi.errorString();
        setLastError(fi.errorString());
        return false;
        // LCOV_EXCL_STOP
    }
    setLastError(QString());
    return true;
}

bool removeBpmFromFile(const QString &fileName) {
    qCDebug(gLogBpmDetect) << "Removing BPM metadata from file:" << fileName;
    AVFormatContext *fmt_ctx = nullptr;
    auto ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        auto errStr = av_errToQString(ret);
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_open_input() returned" << ret << errStr;
        setLastError(errStr);
        return false;
    }
    // Retrieve stream information.
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        // LCOV_EXCL_START
        auto errStr = av_errToQString(ret);
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_find_stream_info() returned" << ret << errStr;
        setLastError(errStr);
        avformat_close_input(&fmt_ctx);
        return false;
    }
    // LCOV_EXCL_STOP
    // Remove BPM from global metadata.
    const auto name = QString::fromUtf8(fmt_ctx->iformat ? fmt_ctx->iformat->name : "");
    const auto key = name.contains(kNameMp3) ? "TBPM" : name.contains(kNameM4a) ? "tmpo" : "bpm";
    qCDebug(gLogBpmDetect) << "Using metadata key:" << key;
    av_dict_set(&fmt_ctx->metadata, key, nullptr, 0);
    // Prepare output file name.
    bool error = false;
    auto outFile = getTemporaryFileName(fileName, &error);
    if (error) {
        // LCOV_EXCL_START
        avformat_close_input(&fmt_ctx);
        return false;
    }
    // LCOV_EXCL_STOP
    // Open output context.
    AVFormatContext *out_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outFile.toUtf8().constData());
    if (ret < 0 || !out_ctx) {
        // LCOV_EXCL_START
        auto errStr = av_errToQString(ret);
        qCCritical(gLogBpmDetect) << "libavformat failed to allocate output context for file:"
                                  << outFile << ". avformat_alloc_output_context2() returned" << ret
                                  << errStr;
        setLastError(errStr);
        avformat_close_input(&fmt_ctx);
        return false;
    }
    // LCOV_EXCL_STOP
    // Copy streams from input to output.
    auto streams = unsafeSpan(fmt_ctx->streams, fmt_ctx->nb_streams);
    for (const auto *in_stream : streams) {
        auto out_stream = avformat_new_stream(out_ctx, nullptr);
        if (!out_stream) {
            // LCOV_EXCL_START
            qCCritical(gLogBpmDetect) << "libavformat failed to create output stream.";
            setLastError(QObject::tr("libavformat failed to create output stream."));
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
            // LCOV_EXCL_STOP
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            // LCOV_EXCL_START
            auto errStr = av_errToQString(ret);
            qCCritical(gLogBpmDetect)
                << "libavformat failed to copy codec parameters." << ret << errStr;
            setLastError(errStr);
            avformat_close_input(&fmt_ctx);
            avformat_free_context(out_ctx);
            return false;
        }
        // LCOV_EXCL_STOP
        out_stream->time_base = in_stream->time_base;
        // Copy stream metadata except TBPM for MP3.
        const auto name = QString::fromUtf8(fmt_ctx->iformat ? fmt_ctx->iformat->name : "");
        auto bpmKey = fmt_ctx->iformat && name.contains(kNameMp3) ? kBpmKeyTBpm :
                      name.contains(kNameM4a)                     ? kBpmKeyTmpo :
                                                                    kBpmKeyBpm;
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
            auto errStr = av_errToQString(ret);
            qCCritical(gLogBpmDetect) << "libavformat failed to open output file:" << outFile
                                      << ". avio_open() returned" << ret << errStr;
            setLastError(errStr);
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
        auto errStr = av_errToQString(ret);
        qCCritical(gLogBpmDetect) << "libavformat failed to write header to output file:" << outFile
                                  << ". avformat_write_header() returned" << ret << errStr;
        setLastError(errStr);
        avformat_close_input(&fmt_ctx);
        if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_ctx->pb);
        }
        avformat_free_context(out_ctx);
        return false;
    }
    // LCOV_EXCL_STOP
    // Write packets (copy mode).
    AVPacket pkt;
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        ret = av_interleaved_write_frame(out_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            // LCOV_EXCL_START
            auto errStr = av_errToQString(ret);
            qCCritical(gLogBpmDetect)
                << "libavformat failed to write frame. av_interleaved_write_frame() returned" << ret
                << errStr;
            setLastError(errStr);
            break;
        }
        // LCOV_EXCL_STOP
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
    QFile fi(fileName);
    if (!fi.remove(fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to delete original file" << fileName << "."
                                  << fi.errorString();
        setLastError(fi.errorString());
        return false;
    }
    // LCOV_EXCL_STOP
    if (!fi.rename(outFile, fileName)) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Failed to replace original file with updated metadata file:"
                                  << fileName << "." << fi.errorString();
        setLastError(fi.errorString());
        return false;
    }
    // LCOV_EXCL_STOP
    setLastError(QString());
    return true;
}

QMap<QString, QVariant> readTagsFromFile(const QString &fileName) {
    AVFormatContext *fmt_ctx = nullptr;
    QMap<QString, QVariant> returnTags;
    static const auto keyArtist = QStringLiteral("artist");
    static const auto keyTitle = QStringLiteral("title");
    static const auto keyLength = QStringLiteral("length");
    static const auto emptyString = QStringLiteral("");
    returnTags.insert(keyArtist, QVariant(emptyString));
    returnTags.insert(keyTitle, QVariant(emptyString));
    returnTags.insert(kBpmKeyBpm, QVariant(0));
    returnTags.insert(keyLength, QVariant(0));
    int ret;
    if ((ret = avformat_open_input(&fmt_ctx, fileName.toUtf8().constData(), nullptr, nullptr)) ==
        0) {
        if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) >= 0) {
            const auto name = QString::fromUtf8(fmt_ctx->iformat ? fmt_ctx->iformat->name : "");
            const auto bpmKey = name.contains(kNameMp3) ? kBpmKeyTBpm :
                                name.contains(kNameM4a) ? kBpmKeyTmpo :
                                                          kBpmKeyBpm;
            qCDebug(gLogBpmDetect) << "Using metadata key for BPM:" << bpmKey;
            const AVDictionaryEntry *e = nullptr;
            while ((e = av_dict_iterate(fmt_ctx->metadata, e))) {
                // LCOV_EXCL_START
                auto key = QString::fromUtf8(e->key).toLower();
                qCDebug(gLogBpmDetect) << "Metadata key:" << key;
                if (key == bpmKey) {
                    returnTags[kBpmKeyBpm] = QString::fromUtf8(e->value).toDouble();
                } else if (key == keyArtist || key == keyTitle) {
                    returnTags[key] = QString::fromUtf8(e->value);
                }
                // LCOV_EXCL_STOP
            }
            if ((!returnTags[kBpmKeyBpm].toInt() || returnTags[keyArtist].toString().isEmpty() ||
                 returnTags[keyTitle].toString().isEmpty()) &&
                fmt_ctx->nb_streams > 0 && fmt_ctx->streams[0] && fmt_ctx->streams[0]->metadata) {
                // Get the first stream's metadata if there is no global metadata.
                e = nullptr;
                while ((e = av_dict_iterate(fmt_ctx->streams[0]->metadata, e))) {
                    // LCOV_EXCL_START
                    auto key = QString::fromUtf8(e->key).toLower();
                    qCDebug(gLogBpmDetect) << "Stream Metadata key:" << key;
                    if (key == bpmKey && !returnTags[kBpmKeyBpm].toInt()) {
                        returnTags[kBpmKeyBpm] = QString::fromUtf8(e->value).toDouble();
                    } else if ((key == keyArtist || key == keyTitle) &&
                               returnTags[key].toString().isEmpty()) {
                        returnTags[key] = QString::fromUtf8(e->value);
                    }
                    // LCOV_EXCL_STOP
                }
            }
            // Read length in milliseconds.
            if (fmt_ctx->duration != AV_NOPTS_VALUE) {
                returnTags[keyLength] =
                    static_cast<qint64>(fmt_ctx->duration / (AV_TIME_BASE / 1000));
            }
        } else {
            // LCOV_EXCL_START
            auto errStr = av_errToQString(ret);
            qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                      << ". avformat_find_stream_info() returned" << ret << errStr;
            setLastError(errStr);
        }
        // LCOV_EXCL_STOP
        avformat_close_input(&fmt_ctx);
    } else {
        auto errStr = av_errToQString(ret);
        qCCritical(gLogBpmDetect) << "libavformat failed to open file:" << fileName
                                  << ". avformat_open_input() returned" << ret << errStr;
        setLastError(errStr);
        return {};
    }
    setLastError(QString());
    return returnTags;
}
