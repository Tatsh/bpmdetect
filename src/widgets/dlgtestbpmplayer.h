// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QThread>
#include <QtMultimedia/QAudioBuffer>
#include <QtMultimedia/QAudioDecoder>
#include <QtMultimedia/QAudioFormat>

class QAudioSink;
class QIODevice;

/** The player for the test dialog. */
class DlgTestBpmPlayer : public QThread {
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
    DlgTestBpmPlayer(const QString file,
                     unsigned int nBeats_,
                     unsigned int bpm_,
                     qint64 posUS_ = 0,
                     QObject *parent = nullptr);
    ~DlgTestBpmPlayer() override;
    /** Get the length in microseconds. */
    auto lengthUs() {
        return lengthUs_;
    }
    /** Stop the player. */
    void stop();
    /**
     * Update the player with new number of beats and position in microseconds.
     * @param nBeats_ New number of beats.
     * @param posUS_ New position in microseconds.
     */
    void update(unsigned int nBeats_, qint64 posUS_ = 0);
    /** Signal for when the player encounters an error. */
    Q_SIGNAL void audioError(QAudio::Error error);
    /** Signal for when length is discovered. */
    Q_SIGNAL void hasLengthUS(qint64 length);

protected:
    void run() override;

protected Q_SLOTS:
    /** Set the error flag. */
    void decodeError(QAudioDecoder::Error error);
    /** Call when decoding is finished. */
    void finishedDecoding();
    /** Handle state change of audio output. */
    void handleStateChange(QAudio::State state);
    /** Read buffer. */
    void readBuffer();

private:
    QAudioBuffer lastBuffer;
    QAudioFormat format;
    QByteArray buffer;
    QAudioDecoder *decoder = nullptr;
    QAudioSink *output = nullptr;
    QIODevice *dev = nullptr;
    char *data = nullptr;
    char *startptr = nullptr;
    qint64 lengthUs_ = 0;
    qint64 posUS = 0;
    qint64 dataRemaining;
    qint64 originalSize;
    unsigned int nBeats = 4;
    float bpm;
    bool readyToPlay = false;
    bool error = false;
};
