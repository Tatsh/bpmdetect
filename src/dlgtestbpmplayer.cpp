#include <QAudioOutput>
#include <QAudioDecoder>

#include "dlgtestbpmplayer.h"

DlgTestBPMPlayer::DlgTestBPMPlayer(const QString file, uint nBeats_, uint bpm_, qint64 posUS_) {
    nBeats = nBeats_;
    bpm = bpm_;
    buffer = new QByteArray();
    decoder = new QAudioDecoder();
    posUS = posUS_;

    connect(decoder, SIGNAL(bufferReady()), this, SLOT(readBuffer()));
    connect(decoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(decodeError(QAudioDecoder::Error)));
    connect(decoder, SIGNAL(finished()), this, SLOT(finishedDecoding()));

    decoder->setSourceFilename(file);
    decoder->start();
}

DlgTestBPMPlayer::~DlgTestBPMPlayer() {
    delete buffer;
    buffer = nullptr;
    delete decoder;
    decoder = nullptr;
    if (output) {
        delete output;
        output = nullptr;
    }
}

void DlgTestBPMPlayer::readBuffer() {
    QAudioBuffer buf = lastBuffer = decoder->read();
    lengthUS += buf.duration();
    buffer->append(buf.data<const char>(), buf.byteCount());
}

void DlgTestBPMPlayer::decodeError(QAudioDecoder::Error err) {
    qDebug() << err;
    error = true;
}

void DlgTestBPMPlayer::finishedDecoding() {
    format = lastBuffer.format();
    if (output) {
        delete output;
        output = nullptr;
    }
    output = new QAudioOutput(format);
    output->setBufferSize(512);
    dev = output->start();
    readyToPlay = true;
    emit hasLengthUS(lengthUS);
}

void DlgTestBPMPlayer::stop() {
    if (output) {
        output->stop();
    }
}

void DlgTestBPMPlayer::update(uint nBeats_, qint64 posUS_) {
    nBeats = nBeats_;
    posUS = posUS_;

    data = startptr = buffer->data();
    const qint64 beatsLength = (qint64)(((60000 * nBeats) / bpm) * 1000);
    const qint32 bytesForBeats = format.bytesForDuration(beatsLength);

    dataRemaining = bytesForBeats * nBeats;
    originalSize = dataRemaining;
    if (posUS > 0) {
        qint32 skipBytes = format.bytesForDuration(posUS);
        if ((data + skipBytes) >= (data + buffer->size())) {
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

    while (dataRemaining) {
        QAudio::State state = output->state();
        if (state != QAudio::ActiveState &&
            state != QAudio::IdleState &&
            state != QAudio::SuspendedState) {
            break;
        }

        qint64 bytesWritten = dev->write(data, dataRemaining);
        dataRemaining -= bytesWritten;
        data += bytesWritten;

        if (dataRemaining <= 0) {
            data = startptr;
            dataRemaining = originalSize;
        }

        usleep(10);
    }
}
