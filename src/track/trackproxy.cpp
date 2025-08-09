/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
     e-mail             : martin.sakmar@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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

using namespace std;
using namespace soundtouch;

TrackProxy::TrackProxy(const char *filename, bool readtags) : Track() {
    m_pTrack = 0;
    m_bConsoleProgress = false;
    setFilename(filename, readtags);
}

TrackProxy::~TrackProxy() {
    if (m_pTrack)
        delete m_pTrack;
    m_pTrack = 0;
}

Track *TrackProxy::createTrack(const char *filename, bool readtags) {
    if (strlen(filename) < 1)
        return 0;

    const char *ext = strrchr(filename, '.');

    if (ext && !strcasecmp(ext, ".wav")) {
        return new TrackWav(filename, readtags);
    }
#ifdef HAVE_VORBISFILE
    if (ext && !strcasecmp(ext, ".ogg")) {
        return new TrackOggVorbis(filename, readtags);
    }
#endif
#ifdef HAVE_MAD
    if (ext && !strcasecmp(ext, ".mp3")) {
        return new TrackMp3(filename, readtags);
    }
#endif
#ifdef HAVE_FLAC
    if (ext &&
        (!strcasecmp(ext, ".flac") || !strcasecmp(ext, ".flc") || !strcasecmp(ext, ".fla"))) {
        return new TrackFlac(filename, readtags);
    }
#endif
#ifdef HAVE_WAVPACK
    if (ext && (!strcasecmp(ext, ".wv") || !strcasecmp(ext, ".wvpk"))) {
        return new TrackWavpack(filename, readtags);
    }
#endif

    return 0;
}

void TrackProxy::setFilename(const char *filename, bool readtags) {
    string strformat = format();
    bool bredetect = redetect();

    if (m_pTrack) {
        close();
        delete m_pTrack;
        m_pTrack = 0;
    }
    m_pTrack = createTrack(filename, readtags);

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

void TrackProxy::setFormat(std::string format) {
    if (m_pTrack)
        m_pTrack->setFormat(format);
}

void TrackProxy::enableConsoleProgress(bool enable) {
    if (m_pTrack)
        m_pTrack->enableConsoleProgress(enable);
    m_bConsoleProgress = enable;
}

void TrackProxy::setStartPos(uint ms) {
    if (m_pTrack)
        m_pTrack->setStartPos(ms);
}

void TrackProxy::setEndPos(uint ms) {
    if (m_pTrack)
        m_pTrack->setEndPos(ms);
}

std::string TrackProxy::filename() const {
    if (m_pTrack)
        return m_pTrack->filename();
    return "";
}

unsigned int TrackProxy::length() const {
    if (m_pTrack)
        return m_pTrack->length();
    return 0;
}

std::string TrackProxy::strLength() {
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

std::string TrackProxy::artist() const {
    if (m_pTrack)
        return m_pTrack->artist();
    return "";
}

std::string TrackProxy::title() const {
    if (m_pTrack)
        return m_pTrack->title();
    return "";
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

std::string TrackProxy::format() const {
    if (m_pTrack)
        return m_pTrack->format();
    return Track::format();
}

void TrackProxy::stop() {
    if (m_pTrack)
        m_pTrack->stop();
}

uint TrackProxy::startPos() const {
    if (m_pTrack)
        return m_pTrack->startPos();
    return 0;
}

uint TrackProxy::endPos() const {
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

void TrackProxy::seek(uint ms) {
    if (m_pTrack)
        m_pTrack->seek(ms);
}

uint TrackProxy::currentPos() {
    if (m_pTrack)
        return m_pTrack->currentPos();
    return 0;
}

int TrackProxy::readSamples(SAMPLETYPE *buffer, unsigned int num) {
    if (m_pTrack)
        return m_pTrack->readSamples(buffer, num);
    return 0;
}

void TrackProxy::storeBPM(string sBPM) {
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

void TrackProxy::printBPM() {
    if (m_pTrack)
        m_pTrack->printBPM();
}

std::string TrackProxy::strBPM() {
    if (m_pTrack)
        return m_pTrack->strBPM();
    return Track::strBPM();
}

std::string TrackProxy::strBPM(std::string format) {
    if (m_pTrack)
        return m_pTrack->strBPM(format);
    return Track::strBPM(format);
}
