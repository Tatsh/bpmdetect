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
#ifdef TESTING
    friend class DlgTestBpmPlayerTest;
#endif
public:
    /**
     * Constructor.
     * @param file File to play.
     * @param nBeats_ Number of beats to loop.
     * @param bpm BPM value.
     * @param posUS Position in microseconds.
     * @param parent Parent object.
     */
    DlgTestBpmPlayer(const QString file,
                     unsigned int nBeats,
                     unsigned int bpm,
                     qint64 posUS = 0,
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
     * @param nBeats New number of beats.
     * @param posUS New position in microseconds.
     */
    void update(unsigned int nBeats, qint64 posUS = 0);
    /** Signal for when the player encounters an error. */
    Q_SIGNAL void audioError(QAudio::Error error);
    /** Signal for when length is discovered. */
    Q_SIGNAL void hasLengthUS(qint64 length);

protected:
    void run() override;
    /** Create audio sink. */
    QAudioSink *audioSinkFactory(const QAudioFormat &format);

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
    QAudioBuffer lastBuffer_;
    QAudioFormat format_;
    QByteArray buffer_;
    QAudioDecoder *decoder_ = nullptr;
    QAudioSink *output_ = nullptr;
    QIODevice *dev_ = nullptr;
    char *data_ = nullptr;
    char *startptr_ = nullptr;
    qint64 lengthUs_ = 0;
    qint64 posUS_ = 0;
    qint64 dataRemaining_;
    qint64 originalSize_;
    unsigned int nBeats_ = 4;
    float bpm_;
    bool readyToPlay_ = false;
    bool error_ = false;
};
