#ifndef DLGTESTBPMPLAYER_H
#define DLGTESTBPMPLAYER_H

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QThread>

class DlgTestBPMPlayer : public QThread {
    Q_OBJECT
public:
    DlgTestBPMPlayer(const QString file, uint nBeats_, uint bpm_, qint64 posUS_ = 0);
    ~DlgTestBPMPlayer();
    const qint64 getLengthUS() { return lengthUS; };
    void update(uint nBeats_, qint64 posUS_ = 0);
    void stop();

protected:
    void run();

protected slots:
    void readBuffer();
    void decodeError(QAudioDecoder::Error);
    void finishedDecoding();

signals:
    void hasLengthUS(const qint64);

private:
    QByteArray *buffer;
    QAudioDecoder *decoder;
    QAudioBuffer lastBuffer;
    QAudioFormat format;
    uint nBeats = 4;
    float bpm;
    qint64 lengthUS = 0;
    bool readyToPlay = false;
    bool error = false;
    qint64 posUS = 0;
    QAudioOutput *output;
    QIODevice *dev;
    qint64 dataRemaining;
    char *data;
    char *startptr;
    qint64 beatsLength;
    qint32 bytesForBeats;
    qint64 originalSize;
};

#endif // DLGTESTBPMPLAYER_H
