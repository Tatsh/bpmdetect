#include <QAudioDecoder>
#include <QAudioSink>
#include <QDebug>
#include <QMediaDevices>
#include <QUrl>

#include "dlgtestbpmplayer.h"

DlgTestBPMPlayer::DlgTestBPMPlayer(
    const QString file, uint nBeats_, uint bpm_, qint64 posUS_, QObject *parent)
    : buffer(QByteArray()), decoder(new QAudioDecoder(this)) {
    nBeats = nBeats_;
    bpm = static_cast<float>(bpm_);
    posUS = posUS_;

    if (parent) {
        setParent(parent);
    }

    decoder->setSource(QUrl::fromLocalFile(file));
    decoder->start();

    connect(decoder, SIGNAL(bufferReady()), this, SLOT(readBuffer()));
    connect(decoder,
            SIGNAL(error(QAudioDecoder::Error)),
            this,
            SLOT(decodeError(QAudioDecoder::Error)));
    connect(decoder, SIGNAL(finished()), this, SLOT(finishedDecoding()));
}

DlgTestBPMPlayer::~DlgTestBPMPlayer() {
}

void DlgTestBPMPlayer::readBuffer() {
    QAudioBuffer buf = lastBuffer = decoder->read();
    lengthUS += buf.duration();
    buffer.append(buf.data<const char>(), buf.byteCount());
}

void DlgTestBPMPlayer::decodeError(QAudioDecoder::Error err) {
    qDebug() << err;
    error = true;
}

void DlgTestBPMPlayer::finishedDecoding() {
    format = lastBuffer.format();
    output = new QAudioSink(format, this);
    dev = output->start();
    readyToPlay = true;
    emit hasLengthUS(lengthUS);
}

void DlgTestBPMPlayer::stop() {
    output->stop();
}

void DlgTestBPMPlayer::update(uint nBeats_, qint64 posUS_) {
    nBeats = nBeats_;
    posUS = posUS_;

    data = startptr = buffer.data();
    const qint64 beatsLength = (qint64)(((60000 * nBeats) / bpm) * 1000);
    const qint32 bytesForBeats = format.bytesForDuration(beatsLength);

    dataRemaining = static_cast<qint64>(bytesForBeats) * nBeats;
    originalSize = dataRemaining;
    if (posUS > 0) {
        qint32 skipBytes = format.bytesForDuration(posUS);
        if ((data + skipBytes) >= (data + buffer.size())) {
            return;
        }

        data += skipBytes;
        startptr = data;
    }
}

void DlgTestBPMPlayer::run() {
    while (!readyToPlay) {
        if (error) {
            return;
        }
        usleep(100);
    }

    update(nBeats);

    while (true) {
        QAudio::State state = output->state();
        if (state != QAudio::ActiveState && state != QAudio::IdleState &&
            state != QAudio::SuspendedState) {
            break;
        }

        qint64 bytesFree = output->bytesFree();
        if (bytesFree > 0) {
            qint64 chunk = qMin(bytesFree, qint64(dataRemaining));
            if (chunk > 0) {
                qint64 bytesWritten = dev->write(data, chunk);
                if (bytesWritten > 0) {
                    dataRemaining -= bytesWritten;
                    data += bytesWritten;
                }
            }
        }

        if (dataRemaining <= 0) {
            data = startptr;
            dataRemaining = originalSize;
        }

        usleep(200);
    }
}
