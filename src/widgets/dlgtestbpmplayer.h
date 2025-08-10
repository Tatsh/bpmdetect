// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#pragma once

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QThread>

class DlgTestBPMPlayer : public QThread {
    Q_OBJECT
public:
    DlgTestBPMPlayer(
        const QString file, uint nBeats_, uint bpm_, qint64 posUS_ = 0, QObject *parent = nullptr);
    ~DlgTestBPMPlayer() override;
    qint64 getLengthUS() {
        return lengthUS;
    }
    void update(uint nBeats_, qint64 posUS_ = 0);
    void stop();

protected:
    void run() override;
    Q_SIGNAL void hasLengthUS(const qint64);

protected Q_SLOTS:
    void readBuffer();
    void decodeError(QAudioDecoder::Error);
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
