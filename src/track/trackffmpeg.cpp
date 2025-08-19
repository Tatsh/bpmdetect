// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioDecoder>

#include "trackffmpeg.h"

TrackFfmpeg::TrackFfmpeg(const QString &fileName, bool readMetadata)
    : Track(), decoder_(new QAudioDecoder(this)) {
    setFileName(fileName, readMetadata);

    QAudioFormat format;
#if defined(SOUNDTOUCH_INTEGER_SAMPLES) && SOUNDTOUCH_INTEGER_SAMPLES
    format.setSampleFormat(QAudioFormat::Int16);
#else
    format.setSampleFormat(QAudioFormat::Float);
#endif
    // These may be ignored but they need to be set so the format is not considered invalid.
    format.setChannelCount(2);
    format.setSampleRate(44100);
    decoder_->setAudioFormat(format);

    connect(decoder_, &QAudioDecoder::bufferReady, this, [this]() {
        auto detector_ = detector();
        if (!isValid() || detector_ == nullptr) {
            qDebug() << "Invalid state for detection.";
            qDebug() << "Detector:" << (detector_ ? "valid" : "nullptr")
                     << ", isValid:" << isValid();
            decoder_->stop();
            return;
        }
        QAudioBuffer buffer;
        if ((buffer = decoder_->read()).isValid()) {
            if (!startedDetection_) {
                qDebug() << "Starting detection with format:" << buffer.format().sampleRate()
                         << "Hz," << buffer.format().channelCount() << "channels.";
                detector_->reset(buffer.format().channelCount(), buffer.format().sampleRate());
                startedDetection_ = true;
            }
            detector_->inputSamples(buffer.constData<soundtouch::SAMPLETYPE>(),
                                    buffer.sampleCount() / buffer.format().channelCount());
            emit progress(decoder_->position(), length());
        }
    });
    connect(decoder_, &QAudioDecoder::sourceChanged, this, [this]() {
        startedDetection_ = false;
        setLength(0);
    });
    connect(decoder_, &QAudioDecoder::durationChanged, this, [this]() {
        if (decoder_->duration() > 0) {
            setLength(decoder_->duration());
            emit hasLength(decoder_->duration());
        }
    });
    connect(decoder_, &QAudioDecoder::finished, this, [this]() {
        auto bpm = correctBpm(detector()->getBpm());
        setBpm(bpm);
        emit hasBpm(bpm);
    });
}

TrackFfmpeg::~TrackFfmpeg() {
    setValid(false);
}

void TrackFfmpeg::readTags() {
}

void TrackFfmpeg::open() {
    close();
    decoder_->setSource(QUrl::fromLocalFile(fileName()));
    if (decoder_->error() == QAudioDecoder::NoError) {
        setValid(true);
        return;
    }
    qCritical() << "Failed to open file:" << fileName() << ", error:" << decoder_->errorString();
}

bpmtype TrackFfmpeg::detectBpm() {
    open();
    auto detector_ = detector();
    if (isValid() && detector_ != nullptr) {
        decoder_->start();
    } else {
        qCritical() << "Invalid state for detection.";
        qCritical() << "Detector:" << (detector_ ? "valid" : "nullptr")
                    << ", isValid:" << isValid();
    }
    return 0;
}

void TrackFfmpeg::storeBpm(const QString &sBpm) {
}

void TrackFfmpeg::removeBpm() {
}
