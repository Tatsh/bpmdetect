// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QThread>

/** The player for the test dialog. */
class DlgTestBPMPlayer : public QThread {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param file File to play.
     * @param nBeats_ Number of beats to loop.
     * @param bpm_ BPM value.
     * @param posUS_ Position in microseconds.
     * @param parent Parent object.
     */
    DlgTestBPMPlayer(
        const QString file, uint nBeats_, uint bpm_, qint64 posUS_ = 0, QObject *parent = nullptr);
    ~DlgTestBPMPlayer() override;
    /** Get the length in microseconds. */
    qint64 getLengthUS() {
        return lengthUS;
    }
    /**
     * Update the player with new number of beats and position in microseconds.
     * @param nBeats_ New number of beats.
     * @param posUS_ New position in microseconds.
     */
    void update(uint nBeats_, qint64 posUS_ = 0);
    /** Stop the player. */
    void stop();
    /** Signal for when length is discovered. */
    Q_SIGNAL void hasLengthUS(qint64);

protected:
    void run() override;

protected Q_SLOTS:
    /** Read buffer. */
    void readBuffer();
    /** Set the error flag. */
    void decodeError(QAudioDecoder::Error);
    /** Call when decoding is finished. */
    void finishedDecoding();

private:
    QByteArray buffer;
    QAudioBuffer lastBuffer;
    QAudioFormat format;
    QAudioDecoder *decoder = nullptr;
    QAudioSink *output = nullptr;
    QIODevice *dev = nullptr;
    char *data = nullptr;
    char *startptr = nullptr;
    qint64 lengthUS = 0;
    qint64 posUS = 0;
    qint64 dataRemaining;
    qint64 originalSize;
    uint nBeats = 4;
    float bpm;
    bool readyToPlay = false;
    bool error = false;
};
