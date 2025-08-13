// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <string>

#include <QSpan>
#include <STTypes.h>

#ifndef NO_GUI
#include <QMutex>
#include <QThread>
#endif

/** Represents a file on the system. */
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

    /** Track type enumeration. */
    enum TRACKTYPE {
        TYPE_UNKNOWN = 0,   //!< Unknown type.
        TYPE_MPEG = 1,      //!< MP3.
        TYPE_WAV = 2,       //!< Wave (RIFF) file.
        TYPE_OGGVORBIS = 3, //!< Ogg Vorbis.
        TYPE_FLAC = 4,      //!< FLAC.
        TYPE_WAVPACK = 5,   //!< WavPack.
    };
    /**
     * Convert QString to BPM.
     * @param sBPM BPM string.
     * @return BPM value.
     */
    static double str2bpm(const QString &sBPM);
    /**
     * Convert BPM to QString using selected format.
     * @param dBPM BPM value.
     * @param format Format (default "0.00", other possible values: "0.0", "0", "000.00", "000.0",
     * "000", "00000").
     * @return Formatted BPM string.
     */
    static QString bpm2str(double dBPM, const QString &format = QStringLiteral("0.00"));
    /**
     * Set minimum BPM.
     * @param dMin Minimum BPM.
     */
    static void setMinBPM(double dMin);
    /**
     * Set maximum BPM.
     * @param dMax Maximum BPM.
     */
    static void setMaxBPM(double dMax);
    /** Get the minimum BPM. */
    static double getMinBPM();
    /** Get the maximum BPM. */
    static double getMaxBPM();
    /** Set limit flag. */
    static void setLimit(bool bLimit);
    /** Clear the BPM. */
    virtual void clearBPM();
    /** Detect the BPM. */
    virtual double detectBPM();
    /** Save the BPM to the metadata of the file. */
    virtual void saveBPM();
    /** Print the BPM to standard output. */
    virtual void printBPM() const;
    /** Set the BPM. */
    virtual void setBPM(double dBPM);
    /** Get the BPM. */
    virtual double getBPM() const;
    /** Get the BPM as a formatted string. */
    virtual QString strBPM() const;
    /** Get the BPM as a string according to the @a format passed in. */
    virtual QString strBPM(const QString &format) const;
    /**
     * Set the filename of the track.
     * @param filename Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    virtual void setFilename(const QString &filename, bool readMetadata = true);
    /** Get the filename. */
    virtual QString filename() const;
    /** Get the track length in miliseconds. */
    virtual unsigned int length() const;
    /** Get the track length as a formatted string. */
    virtual QString strLength() const;
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
    virtual void enableConsoleProgress(bool enable = true);
    /** Stop detection if it is running. */
    virtual void stop();
    /** Set the start position to @a ms. */
    virtual void setStartPos(qint64 ms);
    /** Get the start position (milliseconds). */
    virtual qint64 startPos() const;
    /** Set the end position to @a ms. */
    virtual void setEndPos(qint64 ms);
    /** Get the end position (milliseconds). */
    virtual qint64 endPos() const;
    /** Get the sample rate. */
    virtual int sampleRate() const;
    /** Get the number of bytes per sample. */
    virtual int sampleBytes() const;
    /** Get the number of bits per sample. */
    virtual int sampleBits() const;
    /** Get the number of channels. */
    virtual int channels() const;
    /** Get the track type. */
    virtual int trackType() const;
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
    virtual void seek(qint64 ms) = 0;
    /** Return the current position from which samples will be read (miliseconds). */
    virtual qint64 currentPos() = 0;
    /** Read samples from current position into @a buffer. */
    virtual int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) = 0;
    /** Store @a sBPM into the metadata of the file. */
    virtual void storeBPM(const QString &sBPM) = 0;
    /** Remove BPM metadata from the file. */
    virtual void removeBPM() = 0;
    /** Mark the file validity state. */
    void setValid(bool bValid);
    /** Set if the file is opened. */
    void setOpened(bool opened);
    /** Set the artist. */
    void setArtist(const QString &artist);
    /** Set the title. */
    void setTitle(const QString &title);
    /** Set the length in miliseconds. */
    void setLength(unsigned int msec);
    /** Set the sample rate. */
    void setSampleRate(int sRate);
    /** Set the amount of bytes per sample. This is the bit-depth divided by `8` normally. */
    void setSampleBytes(int bytes);
    /** Set the amount of channels. */
    void setChannels(int channels);
    /** Set the track type. */
    void setTrackType(TRACKTYPE type);

private:
    double correctBPM(double dBPM) const;
    void init();
    void setProgress(double progress);

    QString m_sArtist;
    QString m_sBPMFormat;
    QString m_sFilename;
    QString m_sTitle;
    TRACKTYPE m_eType;
    bool m_bConProgress;
    bool m_bOpened;
    bool m_bRedetect;
    bool m_bStop;
    bool m_bValid;
    double m_dBPM;
    double m_dProgress;
    int m_iChannels;
    int m_iSampleBytes;
    int m_iSamplerate;
    qint64 m_iEndPos;
    qint64 m_iStartPos;
    uint m_iLength;

    static double _dMinBPM;
    static double _dMaxBPM;
    static bool _bLimit;

#ifndef NO_GUI
private:
    QMutex m_qMutex;

protected:
    void run() override;

public:
    /** Start the BPM detection process. */
    void startDetection();
#endif // NO_GUI
};
