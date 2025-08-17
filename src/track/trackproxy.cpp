// SPDX-License-Identifier: GPL-3.0-or-later
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

TrackProxy::TrackProxy(const QString &fileName, bool readMetadata) : Track() {
    m_pTrack = nullptr;
    m_bConsoleProgress = false;
    setFileName(fileName, readMetadata);
}

TrackProxy::~TrackProxy() {
    if (m_pTrack) {
        delete m_pTrack;
        m_pTrack = nullptr;
    }
}

Track *TrackProxy::createTrack(const QString &fileName, bool readMetadata) {
    if (fileName.isEmpty())
        return nullptr;

    const auto dotLastIndex = fileName.lastIndexOf(QStringLiteral("."));
    if (dotLastIndex == -1)
        return nullptr;
    const auto ext = fileName.mid(dotLastIndex).toLower().trimmed();
    if (ext.isEmpty())
        return nullptr;

    if (ext == QStringLiteral(".wav")) {
        return new TrackWav(fileName, readMetadata);
    }
#ifdef HAVE_VORBISFILE
    if (ext == QStringLiteral(".ogg")) {
        return new TrackOggVorbis(fileName, readMetadata);
    }
#endif
#ifdef HAVE_MAD
    if (ext == QStringLiteral(".mp3")) {
        return new TrackMp3(fileName, readMetadata);
    }
#endif
#ifdef HAVE_FLAC
    if (ext == QStringLiteral(".flac") || ext == QStringLiteral(".fla") ||
        ext == QStringLiteral(".flc")) {
        return new TrackFlac(fileName, readMetadata);
    }
#endif
#ifdef HAVE_WAVPACK
    if (ext == QStringLiteral(".wv") || ext == QStringLiteral(".wavpack")) {
        return new TrackWavpack(fileName, readMetadata);
    }
#endif

    return nullptr;
}

void TrackProxy::setFileName(const QString &fileName, bool readMetadata) {
    auto strFormat = format();
    bool bRedetect = redetect();

    if (m_pTrack) {
        close();
        delete m_pTrack;
        m_pTrack = nullptr;
    }
    m_pTrack = createTrack(fileName, readMetadata);

    if (m_pTrack) {
        m_pTrack->setRedetect(bRedetect);
        m_pTrack->setFormat(strFormat);
        m_pTrack->setConsoleProgress(m_bConsoleProgress);
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

bpmtype TrackProxy::detectBpm() {
    if (m_pTrack)
        return m_pTrack->detectBpm();
    return 0;
}

double TrackProxy::progress() {
    if (m_pTrack)
        return m_pTrack->progress();
    return 0;
}

void TrackProxy::setBpm(bpmtype dBpm) {
    if (m_pTrack)
        m_pTrack->setBpm(dBpm);
}

void TrackProxy::setRedetect(bool redetect) {
    if (m_pTrack)
        m_pTrack->setRedetect(redetect);
}

void TrackProxy::setFormat(const QString &format) {
    if (m_pTrack)
        m_pTrack->setFormat(format);
}

void TrackProxy::setConsoleProgress(bool enable) {
    if (m_pTrack)
        m_pTrack->setConsoleProgress(enable);
    m_bConsoleProgress = enable;
}

void TrackProxy::setStartPos(quint64 ms) {
    if (m_pTrack)
        m_pTrack->setStartPos(ms);
}

void TrackProxy::setEndPos(quint64 ms) {
    if (m_pTrack)
        m_pTrack->setEndPos(ms);
}

QString TrackProxy::fileName() const {
    if (m_pTrack)
        return m_pTrack->fileName();
    return QStringLiteral("");
}

quint64 TrackProxy::length() const {
    if (m_pTrack)
        return m_pTrack->length();
    return 0;
}

QString TrackProxy::formattedLength() const {
    if (m_pTrack)
        return m_pTrack->formattedLength();
    return Track::formattedLength();
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

quint64 TrackProxy::startPos() const {
    if (m_pTrack)
        return m_pTrack->startPos();
    return 0;
}

quint64 TrackProxy::endPos() const {
    if (m_pTrack)
        return m_pTrack->endPos();
    return 0;
}

unsigned int TrackProxy::sampleRate() const {
    if (m_pTrack)
        return m_pTrack->sampleRate();
    return 0;
}

unsigned int TrackProxy::sampleBytes() const {
    if (m_pTrack)
        return m_pTrack->sampleBytes();
    return 0;
}

unsigned int TrackProxy::sampleBits() const {
    if (m_pTrack)
        return m_pTrack->sampleBits();
    return 0;
}

unsigned int TrackProxy::channels() const {
    if (m_pTrack)
        return m_pTrack->channels();
    return 0;
}

Track::TrackType TrackProxy::trackType() const {
    if (m_pTrack)
        return m_pTrack->trackType();
    return Unknown;
}

void TrackProxy::open() {
    if (m_pTrack)
        m_pTrack->open();
}

void TrackProxy::close() {
    if (m_pTrack)
        m_pTrack->close();
}

void TrackProxy::seek(quint64 ms) {
    if (m_pTrack)
        m_pTrack->seek(ms);
}

quint64 TrackProxy::currentPos() {
    if (m_pTrack)
        return m_pTrack->currentPos();
    return 0;
}

int TrackProxy::readSamples(QSpan<soundtouch::SAMPLETYPE> sp) {
    if (m_pTrack)
        return m_pTrack->readSamples(sp);
    return 0;
}

void TrackProxy::storeBpm(const QString &sBpm) {
    if (m_pTrack)
        m_pTrack->storeBpm(sBpm);
}

void TrackProxy::removeBpm() {
    if (m_pTrack)
        m_pTrack->removeBpm();
}

bpmtype TrackProxy::bpm() const {
    if (m_pTrack)
        return m_pTrack->bpm();
    return 0;
}

void TrackProxy::clearBpm() {
    if (m_pTrack)
        m_pTrack->clearBpm();
}

void TrackProxy::saveBpm() {
    if (m_pTrack)
        m_pTrack->saveBpm();
}

void TrackProxy::printBpm() const {
    if (m_pTrack)
        m_pTrack->printBpm();
}

QString TrackProxy::formatted() const {
    if (m_pTrack)
        return m_pTrack->formatted();
    return Track::formatted();
}

QString TrackProxy::formatted(const QString &format) const {
    if (m_pTrack)
        return m_pTrack->formatted(format);
    return Track::formatted(format);
}
