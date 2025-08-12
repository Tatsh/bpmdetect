// SPDX-License-Identifier: GPL-3.0-or-later
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

    connect(decoder, &QAudioDecoder::bufferReady, this, &DlgTestBPMPlayer::readBuffer);
    connect(decoder,
            QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
            this,
            &DlgTestBPMPlayer::decodeError);
    connect(decoder, &QAudioDecoder::finished, this, &DlgTestBPMPlayer::finishedDecoding);
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
    const auto beatsLength =
        static_cast<qint64>(((60000.0f * static_cast<float>(nBeats)) / bpm) * 1000.0f);
    const auto bytesForBeats = format.bytesForDuration(beatsLength);

    dataRemaining = static_cast<qint64>(bytesForBeats) * nBeats;
    originalSize = dataRemaining;
    if (posUS > 0) {
        auto skipBytes = format.bytesForDuration(posUS);
        if (skipBytes >= buffer.size()) {
            return;
        }
#pragma clang unsafe_buffer_usage begin
        // This should be left using raw pointers to avoid performance problems.
        data += skipBytes;
#pragma clang unsafe_buffer_usage end
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
        auto state = output->state();
        if (state != QAudio::ActiveState && state != QAudio::IdleState &&
            state != QAudio::SuspendedState) {
            break;
        }

        auto bytesFree = output->bytesFree();
        if (bytesFree > 0) {
            auto chunk = qMin(bytesFree, dataRemaining);
            if (chunk > 0) {
                auto bytesWritten = dev->write(data, chunk);
                if (bytesWritten > 0) {
                    dataRemaining -= bytesWritten;
#pragma clang unsafe_buffer_usage begin
                    // This should be left using raw pointers to avoid performance problems.
                    data += bytesWritten;
#pragma clang unsafe_buffer_usage end
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
