// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QSpan>
#include <STTypes.h>

#ifndef NO_GUI
#include <QtCore/QMutex>
#include <QtCore/QThread>
#endif

#include "soundtouchbpmdetector.h"
#include "utils.h"

namespace TagLib {
    class FileRef;
}

/** Represents a file on the system. */
class Track
#ifndef NO_GUI
    : public QThread
#endif
{
    friend class TrackProxy;
#ifdef TESTING
    friend class TrackTest;
#endif

public:
    virtual ~Track()
#ifndef NO_GUI
        override
#endif
        ;

    /** Track type enumeration. */
    enum TrackType {
        Unknown = 0, //!< Unknown type.
        Flac,        //!< FLAC.
        Mp3,         //!< MP3.
        Ogg,         //!< Ogg Vorbis.
        WavPack,     //!< WavPack.
        Wave,        //!< Wave (RIFF) file.
    };
    /**
     * Set minimum BPM.
     * @param dMin Minimum BPM.
     */
    static void setMinimumBpm(bpmtype dMin);
    /**
     * Set maximum BPM.
     * @param dMax Maximum BPM.
     */
    static void setMaximumBpm(bpmtype dMax);
    /** Get the minimum BPM. */
    static bpmtype minimumBpm();
    /** Get the maximum BPM. */
    static bpmtype maximumBpm();
    /** Clear the BPM. */
    virtual void clearBpm();
    /** Detect the BPM. */
    virtual bpmtype detectBpm();
    /** Save the BPM to the metadata of the file. */
    virtual void saveBpm();
    /** Print the BPM to standard output. */
    virtual void printBpm() const;
    /** Set the BPM. */
    virtual void setBpm(bpmtype dBpm);
    /** Get the BPM. */
    virtual bpmtype bpm() const;
    /** Get the BPM as a formatted string. */
    virtual QString formatted() const;
    /** Get the BPM as a string according to the @a format passed in. */
    virtual QString formatted(const QString &format) const;
    /**
     * Set the fileName of the track.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    virtual void setFileName(const QString &fileName, bool readMetadata = true);
    /** Get the fileName. */
    virtual QString fileName() const;
    /** Get the track length in miliseconds. */
    virtual quint64 length() const;
    /** Get the track length as a formatted string. */
    virtual QString formattedLength() const;
    /** Check if the track is valid. */
    virtual bool isValid() const;
    /** Check if the file is opened. */
    virtual bool isOpened() const;
    /** Set the track artist. */
    virtual QString artist() const;
    /** Get the track title. */
    virtual QString title() const;
    /** Set if detection should recur. */
    virtual void setRedetect(bool redetect);
    /** Get the redetection flag. */
    virtual bool redetect() const;
    /** Get the progress value. */
    virtual double progress() const;
    /** Set the format of the BPM string. */
    virtual void setFormat(const QString &format = QStringLiteral("0.00"));
    /** Get the BPM format. */
    virtual QString format() const;
    /** Enable console progress. */
    virtual void setConsoleProgress(bool enable = true);
    /** Stop detection if it is running. */
    virtual void stop();
    /** Set the start position to @a ms. */
    virtual void setStartPos(quint64 ms);
    /** Get the start position (milliseconds). */
    virtual quint64 startPos() const;
    /** Set the end position to @a ms. */
    virtual void setEndPos(quint64 ms);
    /** Get the end position (milliseconds). */
    virtual quint64 endPos() const;
    /** Get the sample rate. */
    virtual unsigned int sampleRate() const;
    /** Get the number of bytes per sample. */
    virtual unsigned int sampleBytes() const;
    /** Get the number of bits per sample. */
    virtual unsigned int sampleBits() const;
    /** Get the number of channels. */
    virtual unsigned int channels() const;
    /** Get the track type. */
    virtual TrackType trackType() const;
    /** Read tags (artist, title, BPM). */
    virtual void readTags() = 0;
    /** Read track information. */
    virtual void readInfo();

protected:
    Track();
    /** Open the track. */
    virtual void open() {
        setOpened(true);
    }
    /** Close the track. */
    virtual void close() {
        setOpened(false);
    }
    /** Seek to @a ms miliseconds. */
    virtual void seek(quint64 ms) = 0;
    /** Return the current position from which samples will be read (miliseconds). */
    virtual quint64 currentPos() = 0;
    /** Read samples from current position into @a buffer. */
    virtual int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) = 0;
    /** Store @a sBpm into the metadata of the file. */
    virtual void storeBpm(const QString &sBpm) = 0;
    /** Remove BPM metadata from the file. */
    virtual void removeBpm() = 0;
    /** Mark the file validity state. */
    void setValid(bool bValid);
    /** Set if the file is opened. */
    void setOpened(bool opened);
    /** Set the artist. */
    void setArtist(const QString &artist);
    /** Set the title. */
    void setTitle(const QString &title);
    /** Set the length in miliseconds. */
    void setLength(quint64 msec);
    /** Set the sample rate. */
    void setSampleRate(unsigned int sRate);
    /** Set the amount of bytes per sample. This is the bit-depth divided by `8` normally. */
    void setSampleBytes(unsigned int bytes);
    /** Set the amount of channels. */
    void setChannels(unsigned int channels);
    /** Set the track type. */
    void setTrackType(TrackType type);
    /** Set BPM detector. */
    void setDetector(AbstractBpmDetector *detector);

private:
    bpmtype correctBpm(bpmtype dBpm) const;
    void setProgress(double progress);

    AbstractBpmDetector *m_detector = nullptr;
    QString m_sArtist;
    QString m_sBpmFormat = QStringLiteral("0.00");
    QString m_sFilename = QStringLiteral("");
    QString m_sTitle;
    TrackType m_eType = Unknown;
    bool m_bConProgress = false;
    bool m_bOpened = false;
    bool m_bRedetect = false;
    bool m_bStop = false;
    bool m_bValid = false;
    bpmtype m_dBpm = 0;
    double m_dProgress = 0;
    quint64 m_iEndPos = 0;
    quint64 m_iLength = 0;
    quint64 m_iStartPos = 0;
    unsigned int m_iChannels = 0;
    unsigned int m_iSampleBytes = 0;
    unsigned int m_iSamplerate = 0;

    static bool _bLimit;
    static bpmtype _dMaxBpm;
    static bpmtype _dMinBpm;

#ifndef NO_GUI
public:
    /** Start the BPM detection process. */
    void startDetection();

protected:
    void run() override;

private:
    QMutex m_qMutex;
#endif // NO_GUI
};
