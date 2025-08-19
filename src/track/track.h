// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QSpan>
#include <STTypes.h>

#include "soundtouchbpmdetector.h"
#include "utils.h"

namespace TagLib {
    class FileRef;
}

/** Represents a file on the system. */
class Track : public QObject {
    Q_OBJECT
#ifdef TESTING
    friend class TrackTest;
#endif

public:
    virtual ~Track() override;
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
    virtual bpmtype detectBpm() = 0;
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
    /** Set the format of the BPM string. */
    virtual void setFormat(const QString &format = QStringLiteral("0.00"));
    /** Get the BPM format. */
    virtual QString format() const;
    /** Stop detection if it is running. */
    virtual void stop() = 0;
    /** Read tags (artist, title, BPM). */
    virtual void readTags() = 0;
    /** Set BPM detector. */
    void setDetector(AbstractBpmDetector *detector);
    /** Get the BPM detector. */
    AbstractBpmDetector *detector() const;

protected:
    explicit Track(QObject *parent = nullptr);
    /** Open the track. */
    virtual void open() = 0;
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
    /** Correct the BPM based on the global minimum and maximum. */
    bpmtype correctBpm(bpmtype dBpm) const;

private:
    AbstractBpmDetector *m_detector = nullptr;
    QString m_sArtist;
    QString m_sBpmFormat = QStringLiteral("0.00");
    QString m_sFilename = QStringLiteral("");
    QString m_sTitle;
    bool m_bOpened = false;
    bool m_bRedetect = false;
    bool m_bValid = false;
    bpmtype m_dBpm = 0;
    quint64 m_iLength = 0;

    static bpmtype _dMaxBpm;
    static bpmtype _dMinBpm;
};
