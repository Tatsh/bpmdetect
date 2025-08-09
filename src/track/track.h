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

#pragma once

#include <string>

#include "core/STTypes.h"

#ifndef NO_GUI
#include <QMutex>
#include <QThread>
#endif

class Track
#ifndef NO_GUI
    : public QThread
#endif
{
    friend class TrackProxy;

public:
    virtual ~Track()
#ifndef NO_GUI
        override
#endif
        ;

    enum TRACKTYPE {
        TYPE_UNKNOWN = 0,
        TYPE_MPEG = 1,
        TYPE_WAV = 2,
        TYPE_OGGVORBIS = 3,
        TYPE_FLAC = 4,
        TYPE_WAVPACK = 5,
    };
    /// Convert std::string to BPM
    static double str2bpm(std::string sBPM);
    /// Convert BPM to std::string using selected format
    static std::string bpm2str(double dBPM, std::string format = "0.00");
    static void setMinBPM(double dMin);
    static void setMaxBPM(double dMax);
    static double getMinBPM();
    static double getMaxBPM();
    static void setLimit(bool bLimit);
    static bool getLimit();

    virtual void clearBPM();
    virtual double detectBPM();
    virtual void saveBPM();
    virtual void printBPM();
    virtual void setBPM(double dBPM);
    virtual double getBPM() const;
    virtual std::string strBPM();
    virtual std::string strBPM(std::string format);

    virtual void setFilename(const char *filename, bool readtags = true);
    void setFilename(std::string filename, bool readtags = true);
    virtual std::string filename() const;

    /// Get track length in miliseconds
    virtual unsigned int length() const;
    virtual std::string strLength();
    virtual bool isValid() const;
    virtual bool isOpened() const;
    virtual std::string artist() const;
    virtual std::string title() const;
    virtual void setRedetect(bool redetect);
    virtual bool redetect() const;
    virtual double progress() const;
    virtual void setFormat(std::string format = "0.00");
    virtual std::string format() const;
    virtual void enableConsoleProgress(bool enable = true);

    /// Stop detection if running
    virtual void stop();

    virtual void setStartPos(uint ms);
    virtual uint startPos() const;
    virtual void setEndPos(uint ms);
    virtual uint endPos() const;
    virtual int samplerate() const;
    virtual int sampleBytes() const;
    virtual int sampleBits() const;
    virtual int channels() const;
    virtual int trackType() const;
    /// Read tags (artist, title, bpm)
    virtual void readTags() = 0;
    virtual void readInfo();

protected:
    Track();
    /// Open the track (filename set by setFilename)
    virtual void open() {
        setOpened(true);
    }
    /// Close the track
    virtual void close() {
        setOpened(false);
    }
    /// Seek to @a ms miliseconds
    virtual void seek(uint ms) = 0;
    /// Return the current position from which samples will be read (miliseconds)
    virtual uint currentPos() = 0;
    /// Read @a num samples from current position into @a buffer
    virtual int readSamples(soundtouch::SAMPLETYPE *buffer, unsigned int num) = 0;
    /// Store @a sBPM into tag
    virtual void storeBPM(std::string sBPM) = 0;
    /// Remove BPM from tag
    virtual void removeBPM() = 0;

    void init();
    double correctBPM(double dBPM);
    void setValid(bool bValid);
    void setOpened(bool opened);
    void setArtist(std::string artist);
    void setTitle(std::string title);
    void setLength(unsigned int msec);
    void setSamplerate(int samplerate);
    void setSampleBytes(int bytes);
    void setChannels(int channels);
    void setTrackType(TRACKTYPE type);
    void setProgress(double progress);

private:
    std::string m_sFilename;
    std::string m_sArtist;
    std::string m_sTitle;
    double m_dBPM;
    double m_dProgress;
    int m_iSamplerate;
    int m_iSampleBytes;
    int m_iChannels;
    uint m_iLength;
    uint m_iStartPos;
    uint m_iEndPos;
    TRACKTYPE m_eType;
    std::string m_sBPMFormat;
    bool m_bValid;
    bool m_bRedetect;
    bool m_bStop;
    bool m_bConProgress;
    bool m_bOpened;

    static double _dMinBPM;
    static double _dMaxBPM;
    static bool _bLimit;

#ifndef NO_GUI
private:
    QMutex m_qMutex;

protected:
    void run() override;

public:
    void startDetection();
#endif // NO_GUI
};
