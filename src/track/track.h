// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QSpan>
#include <STTypes.h>

#include "soundtouchbpmdetector.h"
#include "utils.h"

class QAudioDecoder;

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
    /**
     * Constructor.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     * @param parent Parent object.
     */
    Track(const QString &fileName, bool readMetadata = true, QObject *parent = nullptr);
    ~Track() override;
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
    void clearBpm();
    /** Detect the BPM. */
    bpmtype detectBpm();
    /** Save the BPM to the metadata of the file. */
    void saveBpm();
    /** Print the BPM to standard output. */
    void printBpm() const;
    /** Set the BPM. */
    void setBpm(bpmtype dBpm);
    /** Get the BPM. */
    bpmtype bpm() const;
    /** Get the BPM as a formatted string. */
    QString formatted() const;
    /** Get the BPM as a string according to the @a format passed in. */
    QString formatted(const QString &format) const;

    /** Get the file name. */
    QString fileName() const;
    /** Get the track length in miliseconds. */
    quint64 length() const;
    /** Get the track length as a formatted string. */
    QString formattedLength() const;
    /** Check if the track is valid. */
    bool isValid() const;
    /** Get the track artist. */
    QString artist() const;
    /** Get the track title. */
    QString title() const;
    /** Set if detection should recur. */
    void setRedetect(bool redetect);
    /** Get the redetection flag. */
    bool redetect() const;
    /** Set the format of the BPM string. */
    void setFormat(const QString &format = QStringLiteral("0.00"));
    /** Get the BPM format. */
    QString format() const;
    /** Stop detection if it is running. */
    void stop();
    /** Read tags (artist, title, BPM). */
    void readTags();
    /** Set BPM detector. */
    void setDetector(AbstractBpmDetector *detector);
    /** Get the BPM detector. */
    AbstractBpmDetector *detector() const;

Q_SIGNALS:
    /**
     * Signal for when BPM has been determined.
     * @param bpm Detected BPM.
     */
    void hasBpm(bpmtype bpm);
    /**
     * Signal for when length has been determined.
     * @param ms Length in milliseconds.
     */
    void hasLength(quint64 ms);
    /**
     * Signal for progress updates.
     * @param pos Current position in milliseconds.
     * @param length Total length in milliseconds.
     */
    void progress(qint64 pos, qint64 length);

protected:
    /** Correct the BPM based on the global minimum and maximum. */
    bpmtype correctBpm(bpmtype dBpm) const;
    /** Open the track. */
    void open();
    /** Store @a sBpm into the metadata of the file. */
    void storeBpm(const QString &sBpm);
    /** Remove BPM metadata from the file. */
    void removeBpm();

private:
    void setFileName(const QString &fileName, bool readMetadata = true);

    AbstractBpmDetector *detector_ = nullptr;
    QAudioDecoder *decoder_ = nullptr;
    QString artist_;
    QString bpmFormat_ = QStringLiteral("0.00");
    QString fileName_ = QStringLiteral("");
    QString title_;
    bool opened_ = false;
    bool redetect_ = false;
    bool valid_ = false;
    bpmtype dBpm_;
    quint64 length_;

    static bpmtype _dMaxBpm;
    static bpmtype _dMinBpm;
};
