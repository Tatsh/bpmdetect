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

#ifndef TRACKOGGVORBIS_H
#define TRACKOGGVORBIS_H

#include "track.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

class TrackOggVorbis : public Track {
public:
  TrackOggVorbis( const char* filename, bool readtags = true );
  TrackOggVorbis( std::string filename, bool readtags = true );
  ~TrackOggVorbis();
  void readTags();

protected:
  void open();
  void close();
  void seek( uint ms );
  uint currentPos();
  int readSamples( soundtouch::SAMPLETYPE* buffer, int num );

  void storeBPM( std::string sBPM );
  void removeBPM();

  int readWavHeaders();
  int readHeaderBlock();
  int readRIFFBlock();
  int checkCharTags();
  int read(char *buffer, int maxElems);
  int read(short *buffer, int maxElems);
  int read(float *buffer, int maxElems);

private:
  unsigned long long m_iCurPosPCM;
  FILE *fptr;
  OggVorbis_File vf;
};

#endif  // TRACKOGGVORBIS_H
