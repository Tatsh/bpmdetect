// SPDX-License-Identifier: GPL-3.0-or-later
#include <cstring>
#include <iostream>

#include "trackproxy.h"
#include "trackwav.h"
#ifdef HAVE_VORBISFILE
#include "trackoggvorbis.h"
#endif
#ifdef HAVE_MAD
#include "trackmp3.h"
#endif
#ifdef HAVE_FLAC
#include "trackflac.h"
#endif
#ifdef HAVE_WAVPACK
#include "trackwavpack.h"
#endif

TrackProxy::TrackProxy(const QString &filename, bool readMetadata) : Track() {
    m_pTrack = nullptr;
    m_bConsoleProgress = false;
    setFilename(filename, readMetadata);
}

TrackProxy::~TrackProxy() {
    if (m_pTrack) {
        delete m_pTrack;
        m_pTrack = nullptr;
    }
}

Track *TrackProxy::createTrack(const QString &filename, bool readMetadata) {
    if (filename.isEmpty())
        return nullptr;

    const auto dotLastIndex = filename.lastIndexOf(QStringLiteral("."));
    if (dotLastIndex == -1)
        return nullptr;
    const auto ext = filename.mid(dotLastIndex).toLower().trimmed();
    if (ext.isEmpty())
        return nullptr;

    if (ext == QStringLiteral(".wav")) {
        return new TrackWav(filename, readMetadata);
    }
#ifdef HAVE_VORBISFILE
    if (ext == QStringLiteral(".ogg")) {
        return new TrackOggVorbis(filename, readMetadata);
    }
#endif
#ifdef HAVE_MAD
    if (ext == QStringLiteral(".mp3")) {
        return new TrackMp3(filename, readMetadata);
    }
#endif
#ifdef HAVE_FLAC
    if (ext == QStringLiteral(".flac") || ext == QStringLiteral(".fla") ||
        ext == QStringLiteral(".flc")) {
        return new TrackFlac(filename, readMetadata);
    }
#endif
#ifdef HAVE_WAVPACK
    if (ext == QStringLiteral(".wv") || ext == QStringLiteral(".wavpack")) {
        return new TrackWavpack(filename, readMetadata);
    }
#endif

    return nullptr;
}

void TrackProxy::setFilename(const QString &filename, bool readMetadata) {
    auto strformat = format();
    bool bredetect = redetect();

    if (m_pTrack) {
        close();
        delete m_pTrack;
        m_pTrack = nullptr;
    }
    m_pTrack = createTrack(filename, readMetadata);

    if (m_pTrack) {
        m_pTrack->setRedetect(bredetect);
        m_pTrack->setFormat(strformat);
        m_pTrack->enableConsoleProgress(m_bConsoleProgress);
    }
}

void TrackProxy::readTags() {
    if (m_pTrack)
        m_pTrack->readTags();
}

void TrackProxy::readInfo() {
    if (m_pTrack)
        m_pTrack->readInfo();
}

double TrackProxy::detectBPM() {
    if (m_pTrack)
        return m_pTrack->detectBPM();
    return 0;
}

double TrackProxy::progress() {
    if (m_pTrack)
        return m_pTrack->progress();
    return 0;
}

void TrackProxy::setBPM(double dBPM) {
    if (m_pTrack)
        m_pTrack->setBPM(dBPM);
}

void TrackProxy::setRedetect(bool redetect) {
    if (m_pTrack)
        m_pTrack->setRedetect(redetect);
}

void TrackProxy::setFormat(const QString &format) {
    if (m_pTrack)
        m_pTrack->setFormat(format);
}

void TrackProxy::enableConsoleProgress(bool enable) {
    if (m_pTrack)
        m_pTrack->enableConsoleProgress(enable);
    m_bConsoleProgress = enable;
}

void TrackProxy::setStartPos(qint64 ms) {
    if (m_pTrack)
        m_pTrack->setStartPos(ms);
}

void TrackProxy::setEndPos(qint64 ms) {
    if (m_pTrack)
        m_pTrack->setEndPos(ms);
}

QString TrackProxy::filename() const {
    if (m_pTrack)
        return m_pTrack->filename();
    return QStringLiteral("");
}

unsigned int TrackProxy::length() const {
    if (m_pTrack)
        return m_pTrack->length();
    return 0;
}

QString TrackProxy::strLength() {
    if (m_pTrack)
        return m_pTrack->strLength();
    return Track::strLength();
}

bool TrackProxy::isValid() const {
    if (m_pTrack)
        return m_pTrack->isValid();
    return false;
}

bool TrackProxy::isOpened() const {
    if (m_pTrack)
        return m_pTrack->isOpened();
    return false;
}

QString TrackProxy::artist() const {
    if (m_pTrack)
        return m_pTrack->artist();
    return QStringLiteral("");
}

QString TrackProxy::title() const {
    if (m_pTrack)
        return m_pTrack->title();
    return QStringLiteral("");
}

bool TrackProxy::redetect() const {
    if (m_pTrack)
        return m_pTrack->redetect();
    return false;
}

double TrackProxy::progress() const {
    if (m_pTrack)
        return m_pTrack->progress();
    return 0;
}

QString TrackProxy::format() const {
    if (m_pTrack)
        return m_pTrack->format();
    return Track::format();
}

void TrackProxy::stop() {
    if (m_pTrack)
        m_pTrack->stop();
}

qint64 TrackProxy::startPos() const {
    if (m_pTrack)
        return m_pTrack->startPos();
    return 0;
}

qint64 TrackProxy::endPos() const {
    if (m_pTrack)
        return m_pTrack->endPos();
    return 0;
}

int TrackProxy::samplerate() const {
    if (m_pTrack)
        return m_pTrack->samplerate();
    return 0;
}

int TrackProxy::sampleBytes() const {
    if (m_pTrack)
        return m_pTrack->sampleBytes();
    return 0;
}

int TrackProxy::sampleBits() const {
    if (m_pTrack)
        return m_pTrack->sampleBits();
    return 0;
}

int TrackProxy::channels() const {
    if (m_pTrack)
        return m_pTrack->channels();
    return 0;
}

int TrackProxy::trackType() const {
    if (m_pTrack)
        return m_pTrack->trackType();
    return TYPE_UNKNOWN;
}

void TrackProxy::open() {
    if (m_pTrack)
        m_pTrack->open();
}

void TrackProxy::close() {
    if (m_pTrack)
        m_pTrack->close();
}

void TrackProxy::seek(qint64 ms) {
    if (m_pTrack)
        m_pTrack->seek(ms);
}

qint64 TrackProxy::currentPos() {
    if (m_pTrack)
        return m_pTrack->currentPos();
    return 0;
}

int TrackProxy::readSamples(QSpan<soundtouch::SAMPLETYPE> sp) {
    if (m_pTrack)
        return m_pTrack->readSamples(sp);
    return 0;
}

void TrackProxy::storeBPM(const QString &sBPM) {
    if (m_pTrack)
        m_pTrack->storeBPM(sBPM);
}

void TrackProxy::removeBPM() {
    if (m_pTrack)
        m_pTrack->removeBPM();
}

double TrackProxy::getBPM() const {
    if (m_pTrack)
        return m_pTrack->getBPM();
    return 0;
}

void TrackProxy::clearBPM() {
    if (m_pTrack)
        m_pTrack->clearBPM();
}

void TrackProxy::saveBPM() {
    if (m_pTrack)
        m_pTrack->saveBPM();
}

void TrackProxy::printBPM() const {
    if (m_pTrack)
        m_pTrack->printBPM();
}

QString TrackProxy::strBPM() const {
    if (m_pTrack)
        return m_pTrack->strBPM();
    return Track::strBPM();
}

QString TrackProxy::strBPM(const QString &format) const {
    if (m_pTrack)
        return m_pTrack->strBPM(format);
    return Track::strBPM(format);
}
