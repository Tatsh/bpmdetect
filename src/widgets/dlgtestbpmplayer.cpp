// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioDecoder>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>
#include <QtWidgets/QMessageBox>

#include "debug.h"
#include "dlgtestbpmplayer.h"

DlgTestBpmPlayer::DlgTestBpmPlayer(const QString file,
                                   unsigned int nBeats_,
                                   unsigned int bpm_,
                                   QAudioDecoder *decoder,
                                   qint64 posUS_,
                                   QObject *parent)
    : QThread(parent), buffer(QByteArray()), decoder_(decoder) {
    nBeats = nBeats_;
    bpm = static_cast<float>(bpm_);
    posUS = posUS_;
    if (!decoder->isSupported()) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Audio decoder is not supported on this platform.";
        emit audioError(QAudio::FatalError);
        return;
        // LCOV_EXCL_STOP
    }

    decoder->setSource(QUrl::fromLocalFile(file));
    decoder->start();

    connect(decoder, &QAudioDecoder::bufferReady, this, &DlgTestBpmPlayer::readBuffer);
    connect(decoder,
            QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
            this,
            &DlgTestBpmPlayer::decodeError);
    connect(decoder, &QAudioDecoder::finished, this, &DlgTestBpmPlayer::finishedDecoding);
}

DlgTestBpmPlayer::~DlgTestBpmPlayer() {
}

void DlgTestBpmPlayer::readBuffer() {
    auto buf = lastBuffer = decoder_->read();
    lengthUs_ += buf.duration();
    buffer.append(buf.data<const char>(), buf.byteCount());
}

void DlgTestBpmPlayer::decodeError(QAudioDecoder::Error err) {
    qCDebug(gLogBpmDetect) << "Audio decoder error:" << err;
    error = true;
}

QAudioSink *DlgTestBpmPlayer::audioSinkFactory(const QAudioFormat &format) {
    return new QAudioSink(format, this);
}

void DlgTestBpmPlayer::finishedDecoding() {
    format_ = lastBuffer.format();
    output = audioSinkFactory(format_);
    connect(output, &QAudioSink::stateChanged, this, &DlgTestBpmPlayer::handleStateChange);
    dev = output->start();
    readyToPlay = true;
    emit hasLengthUS(lengthUs_);
}

// LCOV_EXCL_START
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-default"
void DlgTestBpmPlayer::handleStateChange(QAudio::State newState) {
    switch (newState) {
    case QAudio::ActiveState:
        qCDebug(gLogBpmDetect) << "Audio output is active.";
        break;
    case QAudio::SuspendedState:
        qCDebug(gLogBpmDetect) << "Audio output is suspended.";
        break;
    case QAudio::StoppedState:
        qCDebug(gLogBpmDetect) << "Audio output is stopped.";
        break;
    case QAudio::IdleState:
        qCDebug(gLogBpmDetect) << "Audio output is idle.";
        break;
    }
    if (output->error() != QAudio::NoError) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
        switch (output->error()) {
#pragma clang diagnostic pop
        case QAudio::OpenError:
            qCCritical(gLogBpmDetect)
                << "Audio output error: The audio device could not be opened.";
            break;
        case QAudio::IOError:
            qCCritical(gLogBpmDetect) << "Audio output error: An I/O error occurred.";
            break;
        case QAudio::UnderrunError:
            qCCritical(gLogBpmDetect) << "Audio output error: An underrun occurred.";
            break;
        case QAudio::FatalError:
            qCCritical(gLogBpmDetect) << "Audio output error: A fatal error occurred.";
            break;
        }
        emit audioError(output->error());
    }
}
#pragma clang diagnostic pop
// LCOV_EXCL_STOP

void DlgTestBpmPlayer::stop() {
    if (output) {
        output->stop();
    }
}

void DlgTestBpmPlayer::update(unsigned int nBeats_, qint64 posUS_) {
    nBeats = nBeats_;
    posUS = posUS_;

    data = startptr = buffer.data();
    const auto beatsLength =
        static_cast<qint64>(((60000.0f * static_cast<float>(nBeats)) / bpm) * 1000.0f);
    const auto bytesForBeats = format_.bytesForDuration(beatsLength);

    dataRemaining = static_cast<qint64>(bytesForBeats) * nBeats;
    originalSize = dataRemaining;
    if (posUS > 0) {
        auto skipBytes = format_.bytesForDuration(posUS);
        if (skipBytes >= buffer.size()) {
            // LCOV_EXCL_START
            return;
            // LCOV_EXCL_STOP
        }
#pragma clang unsafe_buffer_usage begin
        // This should be left using raw pointers to avoid performance problems.
        data += skipBytes;
#pragma clang unsafe_buffer_usage end
        startptr = data;
    }
}

void DlgTestBpmPlayer::run() {
    while (!readyToPlay) {
        if (error) {
            // LCOV_EXCL_START
            return;
            // LCOV_EXCL_STOP
        }
        usleep(100);
    }

    update(nBeats);

    while (true) {
        auto state = output->state();
        if (state != QAudio::ActiveState && state != QAudio::IdleState &&
            state != QAudio::SuspendedState) {
            // LCOV_EXCL_START
            break;
            // LCOV_EXCL_STOP
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
