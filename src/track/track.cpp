// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <BPMDetect.h>
#include <fileref.h>
#include <tag.h>

#include "track.h"

double Track::_dMinBPM = 80.;
double Track::_dMaxBPM = 185.;
bool Track::_bLimit = false;

Track::Track() {
    init();
    enableConsoleProgress(false);
    setFilename(QStringLiteral(""), false);
}

Track::~Track() {
    setFilename(QStringLiteral(""));
}

void Track::init() {
    setTrackType(TYPE_UNKNOWN);
    setValid(false);
    setOpened(false);
    setSamplerate(0);
    setSampleBytes(0);
    setChannels(0);
    setBPM(0);
    setProgress(0);
    setLength(0);
    setStartPos(0);
    setEndPos(0);
}

void Track::setMinBPM(double dMin) {
    if (dMin > 30. && dMin < 300.)
        _dMinBPM = dMin;
    // swap min and max if min is greater than max
    if (_dMinBPM > _dMaxBPM) {
        double temp = _dMinBPM;
        _dMinBPM = _dMaxBPM;
        _dMaxBPM = temp;
    }
}

void Track::setMaxBPM(double dMax) {
    if (dMax > 30. && dMax < 300.)
        _dMaxBPM = dMax;
    // swap min and max if min is greater than max
    if (_dMinBPM > _dMaxBPM) {
        double temp = _dMinBPM;
        _dMinBPM = _dMaxBPM;
        _dMaxBPM = temp;
    }
}

double Track::getMinBPM() {
    return _dMinBPM;
}

double Track::getMaxBPM() {
    return _dMaxBPM;
}

void Track::setLimit(bool bLimit) {
    _bLimit = bLimit;
}

double Track::str2bpm(const QString &sBPM) {
    auto BPM = sBPM.toDouble();
    while (BPM > 300)
        BPM = BPM / 10;
    return BPM;
}

QString Track::bpm2str(double dBPM, const QString &format) {
    auto zero = QChar::fromLatin1('0');
    if (format == QStringLiteral("0.0")) {
        return QString::number(dBPM, 'f', 1);
    } else if (format == QStringLiteral("0")) {
        return QString::number(dBPM, 'd', 0);
    } else if (format == QStringLiteral("000.00")) {
        return QString::number(dBPM, 'f', 2).rightJustified(6, zero);
    } else if (format == QStringLiteral("000.0")) {
        return QString::number(dBPM, 'f', 1).rightJustified(5, zero);
    } else if (format == QStringLiteral("000")) {
        return QString::number(dBPM, 'd', 0).rightJustified(3, zero);
    } else if (format == QStringLiteral("00000")) {
        return QString::number(dBPM, 'd', 0).rightJustified(5, zero);
    }
    // all other formats are converted to "0.00"
    return QString::number(dBPM, 'f', 2);
}

QString Track::strBPM() const {
    return bpm2str(getBPM(), format());
}

QString Track::strBPM(const QString &format) const {
    return bpm2str(getBPM(), format);
}

void Track::setFilename(const QString &filename, bool readMetadata) {
#ifndef NO_GUI
    if (isRunning()) {
#ifdef DEBUG
        clog << "setFilename: thread not finished, stopping..." << endl;
#endif
        stop();
        wait();
    }
#endif

    close();
    init();
    m_sFilename = filename;
    if (!filename.isEmpty()) {
        readInfo();
        if (readMetadata)
            readTags();
    }
}

QString Track::filename() const {
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

void Track::setBPM(double dBPM) {
    m_dBPM = dBPM;
}

double Track::getBPM() const {
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

void Track::setStartPos(qint64 ms) {
    if (ms > length())
        return;
    m_iStartPos = ms;
    if (m_iEndPos < m_iStartPos) {
        auto tmp = m_iEndPos;
        m_iEndPos = m_iStartPos;
        m_iStartPos = tmp;
    }
}

qint64 Track::startPos() const {
    return m_iStartPos;
}

void Track::setEndPos(qint64 ms) {
    if (ms > length())
        return;
    m_iEndPos = ms;
    if (m_iEndPos < m_iStartPos) {
        qint64 tmp = m_iEndPos;
        m_iEndPos = m_iStartPos;
        m_iStartPos = tmp;
    }
}

qint64 Track::endPos() const {
    return m_iEndPos;
}

double Track::progress() const {
    return m_dProgress;
}

void Track::setProgress(double progress) {
    if (0 <= progress && progress <= 100)
        m_dProgress = progress;
}

void Track::enableConsoleProgress(bool enable) {
    m_bConProgress = enable;
}

void Track::setTrackType(TRACKTYPE type) {
    m_eType = type;
}

int Track::trackType() const {
    return m_eType;
}

void Track::setSamplerate(int samplerate) {
    m_iSamplerate = samplerate;
}

int Track::samplerate() const {
    return m_iSamplerate;
}

void Track::setSampleBytes(int bytes) {
    if (bytes < 0)
        return;
    if (bytes > 4) {
        if (!(bytes % 8)) {
            bytes = bytes / 8;
        } else {
#ifdef DEBUG
            cerr << "Error: setSampleBytes: " << bytes << endl;
#endif
            return;
        }
    }
    if (bytes > 4)
        return;
    m_iSampleBytes = bytes;
}

int Track::sampleBits() const {
    return 8 * sampleBytes();
}

int Track::sampleBytes() const {
    return m_iSampleBytes;
}

void Track::setChannels(int channels) {
    m_iChannels = channels;
}

int Track::channels() const {
    return m_iChannels;
}

unsigned int Track::length() const {
    return m_iLength;
}

void Track::setLength(unsigned int msec) {
    m_iLength = msec;
}

QString Track::strLength() const {
    uint len = length();

    uint csecs = len / 10;
    uint secs = csecs / 100;
    csecs = csecs % 100;
    uint mins = secs / 60;
    secs = secs % 60;
    auto zero = QChar::fromLatin1('0');

    return QStringLiteral("%1:%2").arg(mins, 2, 10, zero).arg(secs, 2, 10, zero);
}

void Track::saveBPM() {
    auto sBPM = bpm2str(getBPM(), format());
    storeBPM(sBPM);
}

void Track::clearBPM() {
    setBPM(0);
    removeBPM();
}

void Track::readInfo() {
    TagLib::FileRef f(filename().toUtf8().constData());

    TagLib::AudioProperties *ap = nullptr;
    if (!f.isNull())
        ap = f.audioProperties();

    if (ap) {
        setChannels(ap->channels());
        setSamplerate(ap->sampleRate());
        setLength(static_cast<unsigned int>(ap->lengthInSeconds()) * 1000);
        setValid(true);
    }
}

double Track::correctBPM(double dBPM) const {
    double min = getMinBPM();
    double max = getMaxBPM();

    if (dBPM < 1)
        return 0.;
    while (dBPM > max)
        dBPM /= 2.;
    while (dBPM < min)
        dBPM *= 2.;

    if (_bLimit && dBPM > max) {
#ifdef DEBUG
        qDebug() << "BPM not within the limit: " << dBPM << " (" << min << ", " << max << ")";
#endif
        dBPM = 0.;
    }

    return dBPM;
}

void Track::printBPM() const {
    std::cout << filename().toStdString() << ": " << bpm2str(getBPM(), format()).toStdString()
              << " BPM" << std::endl;
}

double Track::detectBPM() {
    open();
    if (!isOpened()) {
#ifdef DEBUG
        qWarning() << "detectBPM: can not open track";
#endif
        return 0;
    }

    setProgress(0);
    m_bStop = false;

    double oldbpm = getBPM();
    const double epsilon = 1e-6;
    if (!redetect() && std::abs(oldbpm) > epsilon) {
        return oldbpm;
    }

    const uint NUMSAMPLES = 4096;
    int chan = channels();
    int srate = samplerate();

#ifdef DEBUG
    cerr << "samplerate: " << srate << ", channels: " << chan << ", sample bits: " << sampleBits()
         << endl;
#endif

    if (!srate || !chan) {
        return oldbpm;
    }
    soundtouch::SAMPLETYPE samples[NUMSAMPLES];

    auto totalsteps = endPos() - startPos();
    soundtouch::BPMDetect bpmd(chan, srate);

    qint64 cprogress = 0, pprogress = 0;
    int readsamples = 0;
    seek(startPos());
    while (!m_bStop && currentPos() < endPos() && 0 < (readsamples = readSamples(samples))) {
        bpmd.inputSamples(samples, readsamples / chan);
        cprogress = currentPos() - startPos();

        setProgress(100. * static_cast<double>(cprogress) / static_cast<double>(totalsteps));
        if (m_bConProgress) {
            while ((100 * cprogress / totalsteps) > pprogress) {
                ++pprogress;
                std::clog << "\r" << (100 * cprogress / totalsteps) << "% " << std::flush;
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
    double BPM = static_cast<double>(bpmd.getBpm());
    BPM = correctBPM(BPM);
    setBPM(BPM);
    setProgress(0);
    close();
    return BPM;
}

void Track::stop() {
    m_bStop = true;
}

#ifndef NO_GUI
void Track::startDetection() {
#ifdef DEBUG
    if (isRunning()) {
        cerr << "Start: thread is running (not starting)" << endl;
        return;
    }
#endif // DEBUG
    start(QThread::IdlePriority);
}

void Track::run() {
    detectBPM();
    setProgress(0);
}

#endif // NO_GUI
