// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtMultimedia/QAudioDecoder>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>

#include "debug.h"
#include "dlgtestbpm.h"
#include "progressbar.h"
#include "track/track.h"

LoopingBuffer::LoopingBuffer(QObject *parent) : QBuffer(parent) {
}

qint64 LoopingBuffer::readData(char *data, qint64 maxlen) {
    if (!isOpen() || buffer() == nullptr || maxlen <= 0) {
        return -1;
    }
    auto bytesRead = QBuffer::readData(data, maxlen);
    if (bytesRead == 0 && pos() >= size()) {
        qCDebug(gLogBpmDetect) << "Looping buffer reached end, seeking to start.";
        seek(0);
        bytesRead = QBuffer::readData(data, maxlen);
    }
    return bytesRead;
}

DlgTestBpm::DlgTestBpm(QString file, bpmtype bpm, QWidget *parent)
    : QDialog(parent), bpm_(bpm), player_(new QMediaPlayer(this)),
      loopingBuffer_(new LoopingBuffer(this)), decoder_(new QAudioDecoder(this)) {
    setupUi(this);
    player_->setLoops(QMediaPlayer::Infinite);
    connect(
        player_, &QMediaPlayer::positionChanged, this->trackPosition, &ProgressBar::setPosition);

    if (file.isEmpty()) {
        // LCOV_EXCL_START
        close();
        // LCOV_EXCL_STOP
    }

    lblBpm->setText(bpmToString(static_cast<bpmtype>(bpm), QStringLiteral("000.00")));
    connect(trackPosition, &ProgressBar::positionChanged, this, &DlgTestBpm::setCustomPos);
    connect(this->cbNBeats, &QComboBox::currentTextChanged, this, &DlgTestBpm::setNumBeats);
    connect(this->btnPos1, &QPushButton::clicked, [this]() { setPosFromButton(1); });
    connect(this->btnPos2, &QPushButton::clicked, [this]() { setPosFromButton(2); });
    connect(this->btnPos3, &QPushButton::clicked, [this]() { setPosFromButton(3); });
    connect(this->btnPos4, &QPushButton::clicked, [this]() { setPosFromButton(4); });

    connect(decoder_, &QAudioDecoder::bufferReady, this, [this]() {
        auto buf = lastBuffer_ = decoder_->read();
        format_ = buf.format();
        lengthUs_ += buf.duration();
        completeBuffer_.append(buf.data<const char>(), buf.byteCount());
    });
    const auto url = QUrl::fromLocalFile(file);
    qCDebug(gLogBpmDetect) << "Decoding file" << file << "from" << url.toString();
    connect(decoder_, &QAudioDecoder::finished, this, [this, &file, &url]() {
        qDebug(gLogBpmDetect) << "Decoding finished, total length (s):" << lengthUs_ / 1000000.0;
        setTrackPositionLength(lengthUs_);
        loopingBuffer_->setData(completeBuffer_);
        loopingBuffer_->open(QIODevice::ReadOnly);
        auto out = new QAudioOutput();
        player_->setAudioOutput(out);
        player_->setSourceDevice(loopingBuffer_, QUrl::fromLocalFile(file));
        qCDebug(gLogBpmDetect) << "Starting playback";
        player_->play();
    });
    decoder_->setSource(url);
    decoder_->start();

    connect(player_, &QMediaPlayer::errorOccurred, [this](QMediaPlayer::Error error) {
        // LCOV_EXCL_START
        if (error != QMediaPlayer::NoError) {
            qCCritical(gLogBpmDetect) << "Media player error occurred:" << player_->errorString();
        }
        // LCOV_EXCL_STOP
    });
    connect(player_, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status) {
        qCDebug(gLogBpmDetect) << "Media status changed to" << status;
    });
    connect(
        player_, &QMediaPlayer::playbackStateChanged, [this](QMediaPlayer::PlaybackState state) {
            qCDebug(gLogBpmDetect) << "Playback state changed to" << state;
        });

    cbNBeats->setEnabled(false);
    btnPos1->setEnabled(false);
    btnPos2->setEnabled(false);
    btnPos3->setEnabled(false);
    btnPos4->setEnabled(false);
}

DlgTestBpm::~DlgTestBpm() {
    if (loopingBuffer_->isOpen()) {
        loopingBuffer_->close();
    }
    player_->stop();
}

void DlgTestBpm::setTrackPositionLength(qint64 length) {
    trackPosition->setLength(static_cast<int>(length) / 1000);
    cbNBeats->setEnabled(true);
    btnPos1->setEnabled(true);
    btnPos2->setEnabled(true);
    btnPos3->setEnabled(true);
    btnPos4->setEnabled(true);
    trackPosition->setEnabled(true);
}

void DlgTestBpm::setPosFromButton(int index) {
    const auto newPos = static_cast<qint64>(static_cast<double>(lengthUs_) * (index * 0.2));
    // m_player->update()
    trackPosition->setPosition((trackPosition->length() * index) / 5);
}

void DlgTestBpm::setCustomPos(int msec) {
    Q_UNUSED(msec)
    const auto newPos = trackPosition->value() * 1000;
    // m_player->update(cbNBeats->currentText().toUInt(), trackPosition->value() * 1000);
}

void DlgTestBpm::setNumBeats(const QString &s) {
    auto value = trackPosition->value();
    if (value < 0) {
        value = 0;
    }
    auto ok = false;
    const auto num = s.toInt(&ok);
    if (!ok || num < 1 || num > 16) {
        return;
    }
    // m_player->update(cbNBeats->currentText().toUInt(), value * 1000);
}
