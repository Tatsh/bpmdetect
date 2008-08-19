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

#ifndef TRACKFMOD_H
#define TRACKFMOD_H

#ifndef HAVE_FMOD
  #error FMOD Ex is required for TrackFMOD (HAVE_FMOD not defined)
#endif

#include "track.h"
#include <fmodex/fmod.h>

class TrackFMOD : public Track {
public:
  static bool initFMODSystem();
  static void closeFMODSystem();
  static FMOD_SYSTEM* getFMODSystem();

  TrackFMOD( const char* filename, bool readtags = true );
  ~TrackFMOD();

  void readTags();

protected:
  void open();
  void close();
  void seek( uint ms );
  uint currentPos();
  int readSamples( soundtouch::SAMPLETYPE* buffer, int num );

  void storeBPM( std::string sBPM );
  void removeBPM();

  void readTagsMPEG( std::string fname );
  void readTagsWAV( std::string fname );
  void readTagsOGG( std::string fname );
  void readTagsFLAC( std::string fname );

  void clearBPMMPEG( std::string fname );
  void clearBPMWAV( std::string fname );
  void clearBPMOGG( std::string fname );
  void clearBPMFLAC( std::string fname );

  void saveBPMMPEG( std::string sBPM, std::string fname );
  void saveBPMWAV( std::string sBPM, std::string fname );
  void saveBPMOGG( std::string sBPM, std::string fname );
  void saveBPMFLAC( std::string sBPM, std::string fname );

  FMOD_SOUND* m_sound;

private:
  unsigned long long m_iCurPosBytes;
};

#endif  // TRACKFMOD_H
