// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <BPMDetect.h>
#include <fileref.h>
#include <tag.h>

#include "track.h"

using namespace std;
using namespace soundtouch;

double Track::_dMinBPM = 80.;
double Track::_dMaxBPM = 185.;
bool Track::_bLimit = false;

Track::Track() {
    init();
    enableConsoleProgress(false);
    setFilename("", false);
}

Track::~Track() {
    setFilename("");
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

bool Track::getLimit() {
    return _bLimit;
}

double Track::str2bpm(string sBPM) {
    double BPM = std::stod(sBPM);
    while (BPM > 300)
        BPM = BPM / 10;
    return BPM;
}

string Track::bpm2str(double dBPM, string format) {
    const uint BPM_LEN = 10;
    char buffer[BPM_LEN];

    if (format == "0.0") {
        snprintf(buffer, BPM_LEN, "%.1f", dBPM);
    } else if (format == "0") {
        snprintf(buffer, BPM_LEN, "%d", static_cast<int>(dBPM));
    } else if (format == "000.00") {
        snprintf(buffer, BPM_LEN, "%06.2f", dBPM);
    } else if (format == "000.0") {
        snprintf(buffer, BPM_LEN, "%05.1f", dBPM);
    } else if (format == "000") {
        snprintf(buffer, BPM_LEN, "%03d", static_cast<int>(dBPM));
    } else if (format == "00000") {
        snprintf(buffer, BPM_LEN, "%05d", static_cast<int>(dBPM * 100));
    } else { // all other formats are converted to "0.00"
        snprintf(buffer, BPM_LEN, "%.2f", dBPM);
    }

    string sBPM = buffer;
    return sBPM;
}

string Track::strBPM() {
    return bpm2str(getBPM(), format());
}

string Track::strBPM(string format) {
    return bpm2str(getBPM(), format);
}

void Track::setFilename(const char *filename, bool readtags) {
    string strfname = filename;
    setFilename(strfname, readtags);
}

void Track::setFilename(string filename, bool readtags) {
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
    if (!filename.empty()) {
        readInfo();
        if (readtags)
            readTags();
    }
}

string Track::filename() const {
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

void Track::setFormat(string format) {
    m_sBPMFormat = format;
}

string Track::format() const {
    return m_sBPMFormat;
}

void Track::setBPM(double dBPM) {
    m_dBPM = dBPM;
}

double Track::getBPM() const {
    return m_dBPM;
}

void Track::setArtist(string artist) {
    m_sArtist = artist;
}

string Track::artist() const {
    return m_sArtist;
}

void Track::setTitle(string title) {
    m_sTitle = title;
}

string Track::title() const {
    return m_sTitle;
}

void Track::setRedetect(bool redetect) {
    m_bRedetect = redetect;
}

bool Track::redetect() const {
    return m_bRedetect;
}

void Track::setStartPos(uint ms) {
    if (ms > length())
        return;
    m_iStartPos = ms;
    if (m_iEndPos < m_iStartPos) {
        uint tmp = m_iEndPos;
        m_iEndPos = m_iStartPos;
        m_iStartPos = tmp;
    }
}

uint Track::startPos() const {
    return m_iStartPos;
}

void Track::setEndPos(uint ms) {
    if (ms > length())
        return;
    m_iEndPos = ms;
    if (m_iEndPos < m_iStartPos) {
        uint tmp = m_iEndPos;
        m_iEndPos = m_iStartPos;
        m_iStartPos = tmp;
    }
}

uint Track::endPos() const {
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

string Track::strLength() {
    uint len = length();

    uint csecs = len / 10;
    uint secs = csecs / 100;
    csecs = csecs % 100;
    uint mins = secs / 60;
    secs = secs % 60;

    const uint TIME_LEN = 20;
    char buffer[TIME_LEN];
    snprintf(buffer, TIME_LEN, "%d:%02d", static_cast<int>(mins), static_cast<int>(secs));
    string timestr = buffer;
    return timestr;
}

void Track::saveBPM() {
    string sBPM = bpm2str(getBPM(), format());
    storeBPM(sBPM);
}

void Track::clearBPM() {
    setBPM(0);
    removeBPM();
}

void Track::readInfo() {
    TagLib::FileRef f(filename().c_str());

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

/**
 * @brief Correct BPM
 * if value is lower than min or greater than max
 * @return corrected BPM
 */
double Track::correctBPM(double dBPM) {
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
        cerr << "BPM not within the limit: " << dBPM << " (" << min << ", " << max << ")" << endl;
#endif
        dBPM = 0.;
    }

    return dBPM;
}

/// Print BPM to stdout
void Track::printBPM() {
    cout << bpm2str(getBPM(), format()) << " BPM" << endl;
}

/**
 * @brief Detect BPM of one track
 * @return detected BPM
 */
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
    SAMPLETYPE samples[NUMSAMPLES];

    uint totalsteps = endPos() - startPos();
    BPMDetect bpmd(chan, srate);

    uint cprogress = 0, pprogress = 0;
    int readsamples = 0;
    seek(startPos());
    while (!m_bStop && currentPos() < endPos() && 0 < (readsamples = readSamples(samples))) {
        bpmd.inputSamples(samples, readsamples / chan);
        cprogress = currentPos() - startPos();

        setProgress(100. * cprogress / static_cast<double>(totalsteps));
        if (m_bConProgress) {
            while ((100 * cprogress / totalsteps) > pprogress) {
                ++pprogress;
                clog << "\r" << (100 * cprogress / totalsteps) << "% " << flush;
            }
        }
    }

    setProgress(100);
    if (m_bConProgress)
        clog << "\r" << flush;

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
