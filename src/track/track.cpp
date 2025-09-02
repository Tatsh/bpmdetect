// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}
#include <BPMDetect.h>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioDecoder>

#include "constants.h"
#include "debug.h"
#include "ffmpegutils.h"
#include "soundtouchbpmdetector.h"
#include "track.h"

bpmtype Track::_dMinBpm = 80.;
bpmtype Track::_dMaxBpm = 185.;

Track::Track(const QString &fileName, QAudioDecoder *decoder, QObject *parent)
    : QObject(parent), decoder_(decoder), fileName_(fileName) {
    isValidFile_ = !fileName_.isEmpty();
    readTags();
    setupDecoder();
}

Track::Track(const QString &fileName, QObject *parent) : QObject(parent), fileName_(fileName) {
    readTags();
}

#ifdef TESTING
Track::Track(QObject *parent) : QObject(parent) {
}
#endif

Track::~Track() {
}

void Track::setupDecoder() {
    // LCOV_EXCL_START
    if (!decoder_) {
        return;
    }
    // LCOV_EXCL_STOP
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
    connect(decoder_, &QAudioDecoder::finished, [this]() {
        decoder_->setSource(QUrl()); // Release the file handle (only an issue on Windows).
        if (stopped_) {
            // LCOV_EXCL_START
            qCDebug(gLogBpmDetect) << "Detection stopped.";
            emit finished();
            return;
            // LCOV_EXCL_STOP
        }
        auto bpm = correctBpm(detector_->getBpm());
        setBpm(bpm);
        if (!hasValidBpm()) {
            // LCOV_EXCL_START
            qCInfo(gLogBpmDetect) << "Invalid BPM detected:" << bpm;
            // LCOV_EXCL_STOP
        } else {
            emit hasBpm(bpm);
        }
        emit finished();
    });
    // LCOV_EXCL_START
    connect(decoder_, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), [this]() {
        decoder_->setSource(QUrl()); // Release the file handle (only an issue on Windows).
        qCCritical(gLogBpmDetect) << "Audio decoder error:" << decoder_->errorString();
        stopped_ = true;
        emit finished();
    });
    // LCOV_EXCL_STOP
}

void Track::setMinimumBpm(bpmtype dMin) {
    if (dMin > 30. && dMin < 300.)
        _dMinBpm = dMin;
    // Swap min and max if min is greater than max.
    if (_dMinBpm > _dMaxBpm) {
        std::swap(_dMinBpm, _dMaxBpm);
    }
}

void Track::setMaximumBpm(bpmtype dMax) {
    if (dMax > 30. && dMax < 300.)
        _dMaxBpm = dMax;
    // Swap min and max if min is greater than max.
    if (_dMinBpm > _dMaxBpm) {
        std::swap(_dMinBpm, _dMaxBpm);
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

void Track::readTags() {
    auto map = readTagsFromFile(fileName_);
    title_ = map[QStringLiteral("title")].toString();
    artist_ = map[QStringLiteral("artist")].toString();
    length_ = map[QStringLiteral("length")].toLongLong();
    dBpm_ = map[QStringLiteral("bpm")].toDouble();
    if (hasValidBpm()) {
        hasSavedBpm_ = true;
    }
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

Track::DetectionState Track::detectBpm() {
    if (isValidFile_ && detector_ != nullptr && decoder_ != nullptr) {
        detector_->reset();
        decoder_->setSource(QUrl::fromLocalFile(fileName_));
        decoder_->start();
    } else {
        qCCritical(gLogBpmDetect) << "Invalid state for detection. Detector:"
                                  << (detector_ ? "valid" : "nullptr")
                                  << ", decoder_:" << (decoder_ ? "valid" : "nullptr")
                                  << ", isValidFile_:" << isValidFile_;
        return Error;
    }
    return Detecting;
}

void Track::storeBpm(const QString &sBpm) {
    hasSavedBpm_ = storeBpmInFile(fileName_, sBpm);
}

bool Track::hasSavedBpm() const {
    return hasSavedBpm_;
}

void Track::removeBpm() {
    hasSavedBpm_ = !removeBpmFromFile(fileName_);
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

quint64 Track::length() const {
    return length_;
}
