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

#include "trackproxy.h"
#include "trackfmod.h"

#include <iostream>

using namespace std;
using namespace soundtouch;

TrackProxy::TrackProxy( const char* filename, bool readtags ) : Track() {
  m_pTrack = 0;
  setFilename(filename, readtags);
}

TrackProxy::~TrackProxy() {
  if(m_pTrack) delete m_pTrack;
}

void TrackProxy::open() {
  if(isValid()) close();
  if(!m_pTrack) m_pTrack = Track::createTrack(filename());
  m_pTrack->open();
  setValid(m_pTrack->isValid());
  setLength(m_pTrack->length());
  setChannels(m_pTrack->channels());
  setSamplerate(m_pTrack->samplerate());
  setSampleBytes(m_pTrack->sampleBytes());
  setTrackType((TRACKTYPE) m_pTrack->trackType());
  m_pTrack->enableConsoleProgress(m_bCProgress);
  m_pTrack->setRedetect(redetect());
  m_pTrack->setFormat(format());
}

void TrackProxy::close() {
  if(m_pTrack) {
    m_pTrack->close();
    delete m_pTrack;
  }
  m_pTrack = 0;
  init();
}

void TrackProxy::seek( uint ms ) {
  if(m_pTrack) m_pTrack->seek(ms);
}

uint TrackProxy::currentPos() {
  if(m_pTrack) return m_pTrack->currentPos();
  return 0;
}

int TrackProxy::readSamples( SAMPLETYPE* buffer, int num ) {
  if(m_pTrack) return m_pTrack->readSamples(buffer, num);
  return 0;
}

void TrackProxy::storeBPM( string sBPM ) {
  if(m_pTrack) m_pTrack->storeBPM(sBPM);
}

void TrackProxy::removeBPM() {
  if(m_pTrack) m_pTrack->removeBPM();
}

void TrackProxy::readTags() {
  if(m_pTrack) {
    m_pTrack->readTags();
    setArtist(m_pTrack->artist());
    setTitle(m_pTrack->title());
    setBPM(m_pTrack->getBPM());
  }
}

void TrackProxy::setBPM( double dBPM ) {
  Track::setBPM(dBPM);
  if(m_pTrack) m_pTrack->setBPM(dBPM);
}

void TrackProxy::setRedetect(bool redetect) {
  Track::setRedetect(redetect);
  if(m_pTrack) m_pTrack->setRedetect(redetect);
}

void TrackProxy::setFormat(std::string format) {
  Track::setFormat(format);
  if(m_pTrack) m_pTrack->setFormat(format);
}

void TrackProxy::enableConsoleProgress(bool enable) {
  m_bCProgress = enable;
  if(m_pTrack) m_pTrack->enableConsoleProgress(enable);
}

void TrackProxy::setStartPos( uint ms ) {
  Track::setStartPos(ms);
  if(m_pTrack) m_pTrack->setStartPos(ms);
}

void TrackProxy::setEndPos( uint ms ) {
  Track::setEndPos(ms);
  if(m_pTrack) m_pTrack->setEndPos(ms);
}

double TrackProxy::progress() {
  if(m_pTrack) return m_pTrack->progress();
  return 0;
}

double TrackProxy::detectBPM() {
  if(m_pTrack) {
    double dBPM = m_pTrack->detectBPM();
    setBPM(dBPM);
    return dBPM;
  }
  return 0;
}

void TrackProxy::stop() {
  Track::stop();
  if(m_pTrack) m_pTrack->stop();
}
