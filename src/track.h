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

class Track
#ifndef NO_GUI
  : public QThread
#endif
{
  friend class TrackProxy;
public:
  virtual ~Track();

  enum TRACKTYPE {
    TYPE_UNKNOWN   = 0,
    TYPE_MPEG      = 1,
    TYPE_WAV       = 2,
    TYPE_OGGVORBIS = 3,
    TYPE_FLAC      = 4,
  };
  /// Convert std::string to BPM
  static double str2bpm( std::string sBPM );
  /// Convert BPM to std::string using selected format
  static std::string bpm2str( double dBPM, std::string format = "0.00");
  static void setMinBPM(double dMin);
  static void setMaxBPM(double dMax);
  static double getMinBPM();
  static double getMaxBPM();
  static void setLimit(bool bLimit);
  static bool getLimit();

  void clearBPM();
  virtual double detectBPM();
  void saveBPM();
  void printBPM();
  virtual void setBPM( double dBPM );
  double getBPM() const;
  std::string strBPM();
  std::string strBPM( std::string format );

  void setFilename( const char* filename, bool readtags = true );
  void setFilename( std::string filename, bool readtags = true );
  std::string filename() const;

  /// Get track length in miliseconds
  unsigned int length() const;
  std::string strLength();
  bool isValid() const;
  std::string artist() const;
  std::string title() const;
  virtual void setRedetect(bool redetect);
  bool redetect() const;
  virtual double progress() const;
  virtual void setFormat(std::string format = "0.00");
  std::string format() const;
  virtual void enableConsoleProgress(bool enable = true);

  /// Stop detection if running
  virtual void stop();

  virtual void setStartPos( uint ms );
  uint startPos() const;
  virtual void setEndPos( uint ms );
  uint endPos() const;
  int samplerate() const;
  int sampleBytes() const;
  int sampleBits() const;
  int channels() const;
  int trackType() const;
  /// Read tags (artist, title, bpm)
  virtual void readTags() = 0;

protected:
  Track();
  /// Open the track (filename set by setFilename)
  virtual void open() {};
  /// Close the track
  virtual void close() {};
  /// Seek to @a ms miliseconds
  virtual void seek( uint ms ) = 0;
  /// Return the current position from which samples will be read (miliseconds)
  virtual uint currentPos() = 0;
  /// Read @a num samples from current position into @a buffer
  virtual int readSamples( soundtouch::SAMPLETYPE* buffer, int num ) = 0;
  /// Store @a sBPM into tag
  virtual void storeBPM( std::string sBPM ) = 0;
  /// Remove BPM from tag
  virtual void removeBPM() = 0;

  void init();
  double correctBPM( double dBPM );
  void setValid( bool bValid );
  void setArtist( std::string artist );
  void setTitle( std::string title );
  void setLength( unsigned int msec );
  void setSamplerate( int samplerate );
  void setSampleBytes( int bytes );
  void setChannels( int channels );
  void setTrackType( TRACKTYPE type );
  void setProgress(double progress);

private:
  std::string m_sFilename;
  std::string m_sArtist;
  std::string m_sTitle;
  double m_dBPM;
  uint m_iLength;
  uint m_iStartPos;
  uint m_iEndPos;
  bool m_bValid;
  bool m_bRedetect;
  bool m_bStop;
  bool m_bConProgress;
  double m_dProgress;
  int m_iSamplerate;
  int m_iSampleBytes;
  int m_iChannels;
  TRACKTYPE m_eType;
  std::string m_sBPMFormat;

  static double _dMinBPM;
  static double _dMaxBPM;
  static bool   _bLimit;

#ifndef NO_GUI
protected:
  void run();

public:
  void startDetection();

private:
  QMutex m_qMutex;
#endif  // NO_GUI
};

#endif  // TRACK_H
