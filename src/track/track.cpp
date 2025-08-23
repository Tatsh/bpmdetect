// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <BPMDetect.h>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioDecoder>
#include <fileref.h>
#include <tag.h>

#include "constants.h"
#include "debug.h"
#include "soundtouchbpmdetector.h"
#include "track.h"

bpmtype Track::_dMinBpm = 80.;
bpmtype Track::_dMaxBpm = 185.;

Track::Track(const QString &fileName,
             QAudioDecoder *const decoder,
             bool readMetadata,
             QObject *parent)
    : QObject(parent), decoder_(decoder) {
    setFileName(fileName, readMetadata);
    setupDecoder();
}

Track::Track(const QString &fileName, QObject *parent) : QObject(parent) {
}

Track::Track(QObject *parent) : QObject(parent) {
    setFileName(QStringLiteral(""), false);
}

Track::~Track() {
}

void Track::setupDecoder() {
    if (!decoder_) {
        return;
    }
    QAudioFormat format;
#if defined(SOUNDTOUCH_INTEGER_SAMPLES) && SOUNDTOUCH_INTEGER_SAMPLES
    format.setSampleFormat(QAudioFormat::Int16);
#else
    format.setSampleFormat(QAudioFormat::Float);
#endif
    format.setChannelCount(DETECTION_CHANNELS);
    format.setSampleRate(DETECTION_SAMPLE_RATE);
    decoder_->setAudioFormat(format);

    connect(decoder_, &QAudioDecoder::bufferReady, [this]() {
        if (!isValid() || detector_ == nullptr) {
            qCDebug(gLogBpmDetect) << "Invalid state for detection.";
            qCDebug(gLogBpmDetect) << "Detector:" << (detector_ ? "valid" : "nullptr")
                                   << ", isValidFile_:" << isValidFile_;
            qCDebug(gLogBpmDetect) << "Stopping decoder.";
            stopped_ = true;
            decoder_->stop();
            return;
        }
        QAudioBuffer buffer;
        if ((buffer = decoder_->read()).isValid()) {
            detector_->inputSamples(buffer.constData<soundtouch::SAMPLETYPE>(),
                                    buffer.frameCount());
        }
    });
    connect(decoder_, &QAudioDecoder::positionChanged, [this](qint64 pos) {
        if (!stopped_) {
            emit progress(pos, length_);
        }
    });
    connect(decoder_, &QAudioDecoder::durationChanged, [this]() {
        if (decoder_->duration() > 0) {
            length_ = decoder_->duration();
            emit hasLength(length_);
        }
    });
    connect(decoder_, &QAudioDecoder::finished, [this]() {
        if (stopped_) {
            qCDebug(gLogBpmDetect) << "Detection stopped.";
            emit finished();
            return;
        }
        auto bpm = correctBpm(detector_->getBpm());
        setBpm(bpm);
        if (!hasValidBpm()) {
            qCInfo(gLogBpmDetect) << "Invalid BPM detected:" << bpm;
        } else {
            emit hasBpm(bpm);
        }
        emit finished();
    });
}

void Track::setMinimumBpm(bpmtype dMin) {
    if (dMin > 30. && dMin < 300.)
        _dMinBpm = dMin;
    // Swap min and max if min is greater than max.
    if (_dMinBpm > _dMaxBpm) {
        auto temp = _dMinBpm;
        _dMinBpm = _dMaxBpm;
        _dMaxBpm = temp;
    }
}

void Track::setMaximumBpm(bpmtype dMax) {
    if (dMax > 30. && dMax < 300.)
        _dMaxBpm = dMax;
    // Swap min and max if min is greater than max.
    if (_dMinBpm > _dMaxBpm) {
        auto temp = _dMinBpm;
        _dMinBpm = _dMaxBpm;
        _dMaxBpm = temp;
    }
}

bpmtype Track::minimumBpm() {
    return _dMinBpm;
}

bpmtype Track::maximumBpm() {
    return _dMaxBpm;
}

QString Track::formatted() const {
    return bpmToString(bpm(), format());
}

QString Track::formatted(const QString &format) const {
    return bpmToString(bpm(), format);
}

void Track::setFileName(const QString &fileName, bool readMetadata) {
    opened_ = false;
    isValidFile_ = false;
    dBpm_ = 0;
    length_ = 0;
    fileName_ = fileName;
    if (!fileName_.isEmpty() && readMetadata) {
        readTags();
    }
}

void Track::readTags() {
    // TODO
}

void Track::open() {
    isValidFile_ = false;
    if (fileName_.isEmpty()) {
        return;
    }
    decoder_->setSource(QUrl::fromLocalFile(fileName_));
    if (decoder_->error() == QAudioDecoder::NoError) {
        isValidFile_ = true;
        return;
    }
    qCCritical(gLogBpmDetect) << "Failed to open file:" << fileName_
                              << ", error:" << decoder_->errorString();
}

QString Track::fileName() const {
    return fileName_;
}

bool Track::isValid() const {
    return isValidFile_;
}

void Track::setFormat(const QString &format) {
    bpmFormat_ = format;
}

QString Track::format() const {
    return bpmFormat_;
}

void Track::setBpm(bpmtype dBpm) {
    dBpm_ = dBpm;
}

bpmtype Track::bpm() const {
    return dBpm_;
}

QString Track::formattedLength() const {
    auto csecs = length_ / 10;
    auto secs = csecs / 100;
    csecs = csecs % 100;
    auto mins = secs / 60;
    secs = secs % 60;
    static const auto zero = QChar::fromLatin1('0');
    return QStringLiteral("%1:%2").arg(mins, 2, 10, zero).arg(secs, 2, 10, zero);
}

QString Track::artist() const {
    return artist_;
}

QString Track::title() const {
    return title_;
}

void Track::saveBpm() {
    storeBpm(bpmToString(bpm(), format()));
}

void Track::clearBpm() {
    setBpm(0);
    removeBpm();
}

void Track::setDetector(AbstractBpmDetector *detector) {
    detector_ = detector;
}

bpmtype Track::correctBpm(bpmtype dBpm) const {
    auto min = minimumBpm();
    auto max = maximumBpm();
    if (dBpm < 1) {
        return 0;
    }
    while (dBpm > max) {
        dBpm /= 2;
    }
    while (dBpm < min) {
        dBpm *= 2;
    }
    return dBpm;
}

void Track::printBpm() const {
    std::cout << fileName().toStdString() << ": " << bpmToString(bpm(), format()).toStdString()
              << " BPM" << std::endl;
}

bpmtype Track::detectBpm() {
    open();
    if (isValidFile_ && detector_ != nullptr && decoder_ != nullptr) {
        detector_->reset();
        decoder_->start();
    } else {
        qCCritical(gLogBpmDetect) << "Invalid state for detection. Detector:"
                                  << (detector_ ? "valid" : "nullptr")
                                  << ", decoder_:" << (decoder_ ? "valid" : "nullptr")
                                  << ", isValidFile_:" << isValidFile_;
    }
    return 0;
}

void Track::storeBpm(const QString &sBpm) {
    // TODO
}

void Track::removeBpm() {
    // TODO
}

void Track::stop() {
    stopped_ = true;
    if (decoder_) {
        decoder_->stop();
    }
}

bool Track::hasValidBpm() const {
    return dBpm_ >= _dMinBpm && dBpm_ <= _dMaxBpm;
}
