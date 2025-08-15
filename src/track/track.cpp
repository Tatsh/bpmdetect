// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <BPMDetect.h>
#include <QDebug>
#include <fileref.h>
#include <tag.h>

#include "track.h"

bpmtype Track::_dMinBPM = 80.;
bpmtype Track::_dMaxBPM = 185.;
bool Track::_bLimit = false;

Track::Track() {
}

Track::~Track() {
}

void Track::setMinimumBpm(bpmtype dMin) {
    if (dMin > 30. && dMin < 300.)
        _dMinBPM = dMin;
    // swap min and max if min is greater than max
    if (_dMinBPM > _dMaxBPM) {
        auto temp = _dMinBPM;
        _dMinBPM = _dMaxBPM;
        _dMaxBPM = temp;
    }
}

void Track::setMaximumBpm(bpmtype dMax) {
    if (dMax > 30. && dMax < 300.)
        _dMaxBPM = dMax;
    // swap min and max if min is greater than max
    if (_dMinBPM > _dMaxBPM) {
        auto temp = _dMinBPM;
        _dMinBPM = _dMaxBPM;
        _dMaxBPM = temp;
    }
}

bpmtype Track::minimumBpm() {
    return _dMinBPM;
}

bpmtype Track::maximumBpm() {
    return _dMaxBPM;
}

void Track::setLimit(bool bLimit) {
    _bLimit = bLimit;
}

QString Track::formatted() const {
    return bpmToString(bpm(), format());
}

QString Track::formatted(const QString &format) const {
    return bpmToString(bpm(), format);
}

void Track::setFileName(const QString &fileName, bool readMetadata) {
#ifndef NO_GUI
    if (isRunning()) {
        qCritical() << "Track thread is running, stopping it before setting fileName.";
        stop();
        wait();
    }
#endif

    m_eType = Unknown;
    m_bValid = false;
    m_bOpened = false;
    m_iSamplerate = 0;
    m_iChannels = 0;
    m_iSampleBytes = 0;
    m_dBPM = 0;
    m_dProgress = 0;
    m_iLength = 0;
    m_iStartPos = 0;
    m_iEndPos = 0;
    close();
    m_sFilename = fileName;
    if (!fileName.isEmpty()) {
        readInfo();
        if (readMetadata)
            readTags();
    }
}

QString Track::fileName() const {
    return m_sFilename;
}

void Track::setValid(bool bValid) {
    m_bValid = bValid;
}

bool Track::isValid() const {
    return m_bValid;
}

void Track::setOpened(bool opened) {
    m_bOpened = opened;
}

bool Track::isOpened() const {
    return m_bOpened;
}

void Track::setFormat(const QString &format) {
    m_sBPMFormat = format;
}

QString Track::format() const {
    return m_sBPMFormat;
}

void Track::setBpm(bpmtype dBPM) {
    m_dBPM = dBPM;
}

bpmtype Track::bpm() const {
    return m_dBPM;
}

void Track::setArtist(const QString &artist) {
    m_sArtist = artist;
}

QString Track::artist() const {
    return m_sArtist;
}

void Track::setTitle(const QString &title) {
    m_sTitle = title;
}

QString Track::title() const {
    return m_sTitle;
}

void Track::setRedetect(bool redetect) {
    m_bRedetect = redetect;
}

bool Track::redetect() const {
    return m_bRedetect;
}

void Track::setStartPos(quint64 ms) {
    if (ms > length())
        return;
    m_iStartPos = ms;
    if (m_iEndPos < m_iStartPos) {
        auto tmp = m_iEndPos;
        m_iEndPos = m_iStartPos;
        m_iStartPos = tmp;
    }
}

quint64 Track::startPos() const {
    return m_iStartPos;
}

void Track::setEndPos(quint64 ms) {
    if (ms > length())
        return;
    m_iEndPos = ms;
    if (m_iEndPos < m_iStartPos) {
        auto tmp = m_iEndPos;
        m_iEndPos = m_iStartPos;
        m_iStartPos = tmp;
    }
}

quint64 Track::endPos() const {
    return m_iEndPos;
}

double Track::progress() const {
    return m_dProgress;
}

void Track::setProgress(double progress) {
    if (0 <= progress && progress <= 100)
        m_dProgress = progress;
}

void Track::setConsoleProgress(bool enable) {
    m_bConProgress = enable;
}

void Track::setTrackType(TrackType type) {
    m_eType = type;
}

Track::TrackType Track::trackType() const {
    return m_eType;
}

void Track::setSampleRate(unsigned int sRate) {
    m_iSamplerate = sRate;
}

unsigned int Track::sampleRate() const {
    return m_iSamplerate;
}

void Track::setSampleBytes(unsigned int bytes) {
    if (bytes > 4) {
        if (!(bytes % 8)) {
            bytes = bytes / 8;
        } else {
            qWarning() << "Not divisible by 8:" << bytes;
            return;
        }
    }
    if (bytes > 4)
        return;
    m_iSampleBytes = bytes;
}

unsigned int Track::sampleBits() const {
    return 8 * sampleBytes();
}

unsigned int Track::sampleBytes() const {
    return m_iSampleBytes;
}

void Track::setChannels(unsigned int channels) {
    m_iChannels = channels;
}

unsigned int Track::channels() const {
    return m_iChannels;
}

quint64 Track::length() const {
    return m_iLength;
}

void Track::setLength(quint64 msec) {
    m_iLength = msec;
}

QString Track::formattedLength() const {
    auto len = length();

    auto csecs = len / 10;
    auto secs = csecs / 100;
    csecs = csecs % 100;
    auto mins = secs / 60;
    secs = secs % 60;
    static const auto zero = QChar::fromLatin1('0');

    return QStringLiteral("%1:%2").arg(mins, 2, 10, zero).arg(secs, 2, 10, zero);
}

void Track::saveBpm() {
    auto sBPM = bpmToString(bpm(), format());
    storeBPM(sBPM);
}

void Track::clearBpm() {
    setBpm(0);
    removeBpm();
}

void Track::readInfo() {
    TagLib::FileRef f(fileName().toUtf8().constData());

    TagLib::AudioProperties *ap = nullptr;
    if (!f.isNull())
        ap = f.audioProperties();

    if (ap) {
        setChannels(static_cast<unsigned int>(ap->channels()));
        setSampleRate(static_cast<unsigned int>(ap->sampleRate()));
        setLength(static_cast<quint64>(ap->lengthInSeconds() * 1000));
        setValid(true);
    }
}

bpmtype Track::correctBPM(bpmtype dBPM) const {
    auto min = minimumBpm();
    auto max = maximumBpm();

    if (dBPM < 1)
        return 0.;
    while (dBPM > max)
        dBPM /= 2.;
    while (dBPM < min)
        dBPM *= 2.;

    if (_bLimit && dBPM > max) {
        qDebug() << "BPM not within the limit: " << dBPM << " (" << min << ", " << max << ")";
        dBPM = 0.;
    }

    return dBPM;
}

void Track::printBpm() const {
    std::cout << fileName().toStdString() << ": " << bpmToString(bpm(), format()).toStdString()
              << " BPM" << std::endl;
}

bpmtype Track::detectBpm() {
    open();
    if (!isOpened()) {
        qCritical() << "Cannot open track";
        return 0;
    }

    setProgress(0);
    m_bStop = false;

    auto oldBPM = bpm();
    const auto epsilon = 1e-6;
    if (!redetect() && std::abs(oldBPM) > epsilon) {
        return oldBPM;
    }

    static const auto NUM_SAMPLES = 4096;
    auto chan = static_cast<int>(channels());

    if (!sampleRate() || !chan) {
        return oldBPM;
    }
    soundtouch::SAMPLETYPE samples[NUM_SAMPLES];

    auto totalSteps = endPos() - startPos();
    soundtouch::BPMDetect detector(chan, static_cast<int>(sampleRate()));

    quint64 currentProgress = 0, pProgress = 0;
    int readSamples_ = 0;
    seek(startPos());
    while (!m_bStop && currentPos() < endPos() && 0 < (readSamples_ = readSamples(samples))) {
        detector.inputSamples(samples, readSamples_ / chan);
        currentProgress = currentPos() - startPos();

        setProgress(100. * static_cast<double>(currentProgress) / static_cast<double>(totalSteps));
        if (m_bConProgress) {
            while ((100 * currentProgress / totalSteps) > pProgress) {
                ++pProgress;
                std::clog << "\r" << (100 * currentProgress / totalSteps) << "% " << std::flush;
            }
        }
    }

    setProgress(100);
    if (m_bConProgress)
        std::clog << "\r" << std::flush;

    if (m_bStop) {
        setProgress(0);
        return 0;
    }
    bpmtype BPM = static_cast<bpmtype>(detector.getBpm());
    BPM = correctBPM(BPM);
    setBpm(BPM);
    setProgress(0);
    close();
    return BPM;
}

void Track::stop() {
    m_bStop = true;
}

#ifndef NO_GUI
void Track::startDetection() {
#ifndef NDEBUG
    if (isRunning()) {
        qCritical() << "Thread is running (not starting).";
        return;
    }
#endif // NDEBUG
    start(QThread::IdlePriority);
}

void Track::run() {
    detectBpm();
    setProgress(0);
}

#endif // NO_GUI
