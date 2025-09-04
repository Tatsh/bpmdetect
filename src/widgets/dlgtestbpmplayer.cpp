// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioDecoder>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>
#include <QtWidgets/QMessageBox>

#include "debug.h"
#include "dlgtestbpmplayer.h"

DlgTestBpmPlayer::DlgTestBpmPlayer(
    const QString file, unsigned int nBeats, bpmtype bpm, qint64 posUS, QObject *parent)
    : QThread(parent), buffer_(QByteArray()), decoder_(new QAudioDecoder(this)) {
    nBeats_ = nBeats;
    bpm_ = bpm;
    posUS_ = posUS;
    if (!decoder_->isSupported()) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "Audio decoder is not supported on this platform.";
        emit audioError(QAudio::FatalError);
        return;
        // LCOV_EXCL_STOP
    }
    connect(decoder_, &QAudioDecoder::bufferReady, this, &DlgTestBpmPlayer::readBuffer);
    connect(decoder_,
            QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
            this,
            &DlgTestBpmPlayer::decodeError);
    connect(decoder_, &QAudioDecoder::finished, this, &DlgTestBpmPlayer::finishedDecoding);
    decoder_->setSource(QUrl::fromLocalFile(file));
    decoder_->start();
}

DlgTestBpmPlayer::~DlgTestBpmPlayer() {
}

void DlgTestBpmPlayer::readBuffer() {
    auto buf = lastBuffer_ = decoder_->read();
    lengthUs_ += buf.duration();
    buffer_.append(buf.data<const char>(), buf.byteCount());
}

void DlgTestBpmPlayer::decodeError(QAudioDecoder::Error err) {
    qCDebug(gLogBpmDetect) << "Audio decoder error:" << err;
    error_ = true;
}

QAudioSink *DlgTestBpmPlayer::audioSinkFactory(const QAudioFormat &format) {
    return new QAudioSink(format, this);
}

void DlgTestBpmPlayer::finishedDecoding() {
    format_ = lastBuffer_.format();
    output_ = audioSinkFactory(format_);
    connect(output_, &QAudioSink::stateChanged, this, &DlgTestBpmPlayer::handleStateChange);
    dev_ = output_->start();
    readyToPlay_ = true;
    emit hasLengthUs(lengthUs_);
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
    if (output_->error() != QAudio::NoError) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
        switch (output_->error()) {
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
        emit audioError(output_->error());
    }
}
#pragma clang diagnostic pop
// LCOV_EXCL_STOP

void DlgTestBpmPlayer::stop() {
    if (output_) {
        output_->stop();
    }
}

void DlgTestBpmPlayer::update(unsigned int nBeats, qint64 posUs) {
    nBeats_ = nBeats;
    posUS_ = posUs;

    data_ = startptr_ = buffer_.data();
    const auto beatsLength =
        static_cast<qint64>(((60000.0 * static_cast<bpmtype>(nBeats_)) / bpm_) * 1000.0);
    const auto bytesForBeats = format_.bytesForDuration(beatsLength);

    dataRemaining_ = static_cast<qint64>(bytesForBeats) * nBeats_;
    originalSize_ = dataRemaining_;
    if (posUS_ > 0) {
        auto skipBytes = format_.bytesForDuration(posUS_);
        if (skipBytes >= buffer_.size()) {
            // LCOV_EXCL_START
            return;
            // LCOV_EXCL_STOP
        }
#pragma clang unsafe_buffer_usage begin
        // This should be left using raw pointers to avoid performance problems.
        data_ += skipBytes;
#pragma clang unsafe_buffer_usage end
        startptr_ = data_;
    }
}

void DlgTestBpmPlayer::setBpm(bpmtype bpm) {
    bpm_ = bpm;
    update(nBeats_, posUS_);
}

void DlgTestBpmPlayer::run() {
    while (!readyToPlay_) {
        if (error_) {
            // LCOV_EXCL_START
            return;
            // LCOV_EXCL_STOP
        }
        usleep(100);
    }

    update(nBeats_);

    while (true) {
        auto state = output_->state();
        if (state != QAudio::ActiveState && state != QAudio::IdleState &&
            state != QAudio::SuspendedState) {
            // LCOV_EXCL_START
            break;
        }

        // This section is excluded because GitHub Actions CI VMs cannot reach here.
        auto bytesFree = output_->bytesFree();
        if (bytesFree > 0) {
            auto chunkSize = qMin(bytesFree, dataRemaining_);
            if (chunkSize > 0) {
                auto bytesWritten = dev_->write(data_, chunkSize);
                if (bytesWritten > 0) {
                    dataRemaining_ -= bytesWritten;
#pragma clang unsafe_buffer_usage begin
                    // This should be left using raw pointers to avoid performance problems.
                    data_ += bytesWritten;
#pragma clang unsafe_buffer_usage end
                }
            }
        }

        if (dataRemaining_ <= 0) {
            data_ = startptr_;
            dataRemaining_ = originalSize_;
        }

        usleep(200);
    }
    // LCOV_EXCL_STOP
}
