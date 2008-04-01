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

#ifndef TRACK_H
#define TRACK_H

#include <string>
#include "STTypes.h"

#ifndef NO_GUI
# include <qthread.h>
# include <qmutex.h>
#endif

#include <fmodex/fmod.h>

enum TRACKTYPE {
  TYPE_UNKNOWN   = 0,
  TYPE_MPEG      = 1,
  TYPE_WAV       = 2,
  TYPE_OGGVORBIS = 3,
  TYPE_FLAC      = 4,
};

/**
*/
class Track
#ifndef NO_GUI
  : public QThread
#endif
{
public:
  Track( std::string filename, bool readtags = false );
  Track( const char* filename, bool readtags = false );
  ~Track();

  /// Convert std::string to BPM
  static double str2bpm( std::string sBPM );
  /// Convert BPM to std::string using selected format
  static std::string bpm2str( double dBPM, std::string format = "0.00");
  static void setMinBPM(double dMin);
  static void setMaxBPM(double dMax);
  static double getMinBPM();
  static double getMaxBPM();

  double detectBPM();
  /// Save BPM to tag formatted using format
  void saveBPM( std::string format = "0.00" );
  /// Clear stored BPM
  void clearBPM();
  /// Print BPM to stdout formatted using format
  void printBPM( std::string format = "0.00" );
  double getBPM() const;
  void setBPM( double dBPM );
  /// Set the filename
  void setFilename( const char* filename, bool readtags = false );
  void setFilename( std::string filename, bool readtags = false );
  std::string getFilename() const;
  /// Get track length in miliseconds
  unsigned int getLength() const;
  /// Return BPM as std::string formatted using format
  std::string strBPM( std::string format = "0.00" );
  std::string strLength();
  bool isValid() const;
  std::string getArtist() const;
  std::string getTitle() const;
  void setRedetect(bool redetect);
  bool getRedetect() const;
  double getProgress() const;
  /// Stop detection if running
  void stop();

  void setStartPos( uint ms );
  uint getStartPos() const;
  void setEndPos( uint ms );
  uint getEndPos() const;
  int getSamplerate() const;
  int getSampleBits() const;
  int getChannels() const;

  TRACKTYPE getTrackType() const;

  virtual void readTags();

protected:
  void init();
  virtual void open();
  virtual void close();
  virtual void seek( uint ms );
  virtual uint currentPos();
  /// Read @a num samples from current position into @a buffer
  virtual int readSamples( soundtouch::SAMPLETYPE* buffer, int num );

  double correctBPM( double dBPM );
  void setValid( bool bValid );
  void setArtist( std::string artist );
  void setTitle( std::string title );
  void setLength( unsigned int msec );
  void setProgress(double progress);
  void setSamplerate( int samplerate );
  void setSampleBits( int bits );
  void setChannels( int channels );
  void setTrackType( TRACKTYPE type );

  void readTagsMPEG();
  void readTagsWAV();
  void readTagsOGG();
  void readTagsFLAC();
// #ifdef HAVE_ID3LIB
  void saveMPEG_ID3( std::string sBPM, std::string filename );
  void saveWAV_ID3( std::string sBPM, std::string filename );
//#endif // HAVE_ID3LIB
  void clearBPMMPEG();
  void clearBPMWAV();
  void clearBPMOGG();
  void clearBPMFLAC();
#ifdef HAVE_TAGLIB
  void saveMPEG_TAG( std::string sBPM, std::string filename );
  void saveWAV_TAG( std::string sBPM, std::string filename );
  void saveOGG_TAG( std::string sBPM, std::string filename );
  void saveFLAC_TAG( std::string sBPM, std::string filename );
#endif // HAVE_TAGLIB

  FMOD_SOUND* m_sound;

private:
  std::string m_sFilename;
  std::string m_sArtist;
  std::string m_sTitle;
  double m_dBPM;
  unsigned int m_iLength;
  unsigned int m_iStartPos;
  unsigned int m_iEndPos;
  bool m_bValid;
  bool m_bRedetect;
  bool m_bStop;
  double m_dProgress;
  
  int m_iSamplerate;
  int m_iSampleBits;
  int m_iChannels;
  TRACKTYPE m_eType;

#ifndef NO_GUI
protected:
  void run();

public:
  void startDetection();
  void setPriority(QThread::Priority priority);

private:
  QMutex m_qMutex;
  QThread::Priority m_iPriority;
#endif
};

#endif
