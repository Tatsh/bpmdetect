// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <string>

#include <QSpan>
#include <STTypes.h>

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
    /// Convert QString to BPM
    static double str2bpm(const QString &sBPM);
    /// Convert BPM to QString using selected format
    static QString bpm2str(double dBPM, QString format = QStringLiteral("0.00"));
    static void setMinBPM(double dMin);
    static void setMaxBPM(double dMax);
    static double getMinBPM();
    static double getMaxBPM();
    static void setLimit(bool bLimit);
    static bool getLimit();

    virtual void clearBPM();
    virtual double detectBPM();
    virtual void saveBPM();
    virtual void printBPM() const;
    virtual void setBPM(double dBPM);
    virtual double getBPM() const;
    virtual QString strBPM() const;
    virtual QString strBPM(const QString &format) const;

    virtual void setFilename(const QString &filename, bool readtags = true);
    virtual QString filename() const;

    /// Get track length in miliseconds
    virtual unsigned int length() const;
    virtual QString strLength();
    virtual bool isValid() const;
    virtual bool isOpened() const;
    virtual QString artist() const;
    virtual QString title() const;
    virtual void setRedetect(bool redetect);
    virtual bool redetect() const;
    virtual double progress() const;
    virtual void setFormat(QString format = QStringLiteral("0.00"));
    virtual QString format() const;
    virtual void enableConsoleProgress(bool enable = true);

    /// Stop detection if running
    virtual void stop();

    virtual void setStartPos(qint64 ms);
    virtual qint64 startPos() const;
    virtual void setEndPos(qint64 ms);
    virtual qint64 endPos() const;
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
    virtual void seek(qint64 ms) = 0;
    /// Return the current position from which samples will be read (miliseconds)
    virtual qint64 currentPos() = 0;
    /// Read @a num samples from current position into @a buffer
    virtual int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) = 0;
    /// Store @a sBPM into tag
    virtual void storeBPM(const QString &sBPM) = 0;
    /// Remove BPM from tag
    virtual void removeBPM() = 0;

    void init();
    double correctBPM(double dBPM) const;
    void setValid(bool bValid);
    void setOpened(bool opened);
    void setArtist(const QString &artist);
    void setTitle(const QString &title);
    void setLength(unsigned int msec);
    void setSamplerate(int samplerate);
    void setSampleBytes(int bytes);
    void setChannels(int channels);
    void setTrackType(TRACKTYPE type);
    void setProgress(double progress);

private:
    QString m_sFilename;
    QString m_sArtist;
    QString m_sTitle;
    double m_dBPM;
    double m_dProgress;
    int m_iSamplerate;
    int m_iSampleBytes;
    int m_iChannels;
    uint m_iLength;
    qint64 m_iStartPos;
    qint64 m_iEndPos;
    TRACKTYPE m_eType;
    QString m_sBPMFormat;
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
