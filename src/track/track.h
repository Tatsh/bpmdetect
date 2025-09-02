// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QSpan>
#include <STTypes.h>

#include "soundtouchbpmdetector.h"
#include "utils.h"

class QAudioDecoder;

/** Represents a file on the system. */
class Track : public QObject {
    Q_OBJECT
#ifdef TESTING
    friend class DlgBpmDetectTest;
    friend class TrackTest;
#endif

public:
    /** Possible states of BPM detection. */
    enum DetectionState {
        Detecting, //!< Currently detecting BPM.
        Error,     //!< Error occurred.
    };
    /**
     * Constructor.
     * @param fileName Filename.
     * @param decoder Audio decoder.
     * @param parent Parent object.
     */
    Track(const QString &fileName, QAudioDecoder *decoder, QObject *parent = nullptr);
    /**
     * Constructor.
     * @param fileName Filename.
     * @param decoder Audio decoder.
     * @param parent Parent object.
     */
    Track(const QString &filename, QObject *parent = nullptr);
#ifdef TESTING
    /**
     * Constructor.
     * @param parent Parent object.
     */
    explicit Track(QObject *parent = nullptr);
#endif
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
    DetectionState detectBpm();
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
    /** Check if the BPM is set and is valid. */
    bool hasValidBpm() const;
    /** If the BPM is saved in the file metadata. */
    bool hasSavedBpm() const;

Q_SIGNALS:
    /**
     * Signal for when BPM has been determined.
     * @param bpm Detected BPM.
     */
    void hasBpm(bpmtype bpm);
    /**
     * Signal for progress updates.
     * @param pos Current position in milliseconds.
     * @param length Total length in milliseconds.
     */
    void progress(qint64 pos, qint64 length);
    /** Signal for when audio is finished being decoded. */
    void finished();

protected:
    /** Correct the BPM based on the global minimum and maximum. */
    bpmtype correctBpm(bpmtype dBpm) const;
    /** Store @a sBpm into the metadata of the file. */
    void storeBpm(const QString &sBpm);
    /** Remove BPM metadata from the file. */
    void removeBpm();

private:
    void setupDecoder();

    AbstractBpmDetector *detector_ = nullptr;
    QAudioDecoder *decoder_ = nullptr;
    QString artist_;
    QString bpmFormat_ = QStringLiteral("0.00");
    QString fileName_;
    QString title_;
    bool hasSavedBpm_ = false;
    bool isValidFile_ = false;
    bool opened_ = false;
    bool stopped_ = false;
    bpmtype dBpm_ = 0;
    quint64 length_ = 0;

    static bpmtype _dMaxBpm;
    static bpmtype _dMinBpm;
};
