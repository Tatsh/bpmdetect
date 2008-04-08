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

#ifndef TRACKPROXY_H
#define TRACKPROXY_H

#include "track.h"

class TrackProxy : public Track {
public:
  TrackProxy( const char* filename, bool readtags = true );
  ~TrackProxy();

  void readTags();
  double detectBPM();
  void stop();
  double progress();
  void setBPM( double dBPM );
  void setRedetect(bool redetect);
  void setFormat(std::string format = "0.00");
  void enableConsoleProgress(bool enable = true);
  void setStartPos( uint ms );
  void setEndPos( uint ms );

protected:
  void open();
  void close();
  void seek( uint ms );
  uint currentPos();
  int readSamples( soundtouch::SAMPLETYPE* buffer, int num );
  void storeBPM( std::string sBPM );
  void removeBPM();

private:
  Track* m_pTrack;
  bool m_bCProgress;
};

#endif  // TRACKPROXY_H
