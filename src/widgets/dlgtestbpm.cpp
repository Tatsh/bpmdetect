// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>

#include "debug.h"
#include "dlgtestbpm.h"
#include "dlgtestbpmplayer.h"
#include "progressbar.h"
#include "track/track.h"

DlgTestBpm::DlgTestBpm(QString file, bpmtype bpm, DlgTestBpmPlayer *player, QWidget *parent)
    : QDialog(parent), player_(player), bpm_(bpm) {
    setupUi(this);

    if (file.isEmpty()) {
        // LCOV_EXCL_START
        close();
        // LCOV_EXCL_STOP
    }

    lblBpm->setValue(bpm);
    connect(trackPosition, &ProgressBar::positionChanged, this, &DlgTestBpm::setCustomPos);
    connect(player_, &DlgTestBpmPlayer::hasLengthUs, this, &DlgTestBpm::setTrackPositionLength);
    connect(player_, &DlgTestBpmPlayer::audioError, [this](QAudio::Error e) {
        // LCOV_EXCL_START
        if (e == QAudio::FatalError) {
            qCCritical(gLogBpmDetect) << "Fatal audio error occurred.";
            reject();
        }
    });
    // LCOV_EXCL_STOP
    connect(this, &QDialog::accepted, player_, &DlgTestBpmPlayer::stop);
    connect(this, &QDialog::rejected, player_, &DlgTestBpmPlayer::stop);
    connect(cbNBeats, &QComboBox::currentTextChanged, this, &DlgTestBpm::setNumBeats);
    connect(btnPos1, &QPushButton::clicked, [this]() { setPosFromButton(1); });
    connect(btnPos2, &QPushButton::clicked, [this]() { setPosFromButton(2); });
    connect(btnPos3, &QPushButton::clicked, [this]() { setPosFromButton(3); });
    connect(btnPos4, &QPushButton::clicked, [this]() { setPosFromButton(4); });
    connect(lblBpm, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v) {
        // LCOV_EXCL_START
        bpm_ = v;
        player_->setBpm(bpm_);
        modifiedBpm_ = true;
    });
    // LCOV_EXCL_STOP

    lblBpm->setEnabled(false);
    cbNBeats->setEnabled(false);
    btnPos1->setEnabled(false);
    btnPos2->setEnabled(false);
    btnPos3->setEnabled(false);
    btnPos4->setEnabled(false);
    trackPosition->setEnabled(false);

#ifndef TESTING
    player_->start();
#endif
}

DlgTestBpm::~DlgTestBpm() {
    if (player_) {
        player_->stop();
    }
}

void DlgTestBpm::setTrackPositionLength(qint64 length) {
    trackPosition->setLength(static_cast<int>(length) / 1000);
    lblBpm->setEnabled(true);
    cbNBeats->setEnabled(true);
    btnPos1->setEnabled(true);
    btnPos2->setEnabled(true);
    btnPos3->setEnabled(true);
    btnPos4->setEnabled(true);
    trackPosition->setEnabled(true);
}

void DlgTestBpm::setPosFromButton(int index) {
    player_->update(cbNBeats->currentText().toUInt(),
                    static_cast<qint64>(static_cast<double>(player_->lengthUs()) * (index * 0.2)));
    trackPosition->setPosition((trackPosition->length() * index) / 5);
}

void DlgTestBpm::setCustomPos(int msec) {
    Q_UNUSED(msec)
    player_->update(cbNBeats->currentText().toUInt(), trackPosition->value() * 1000);
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
    player_->update(cbNBeats->currentText().toUInt(), value * 1000);
}

// LCOV_EXCL_START
void DlgTestBpm::accept() {
    if (lblBpm->hasFocus()) {
        return;
    }
    if (modifiedBpm_) {
        emit newBpmOnClose(bpm_);
    }
    QDialog::accept();
}
// LCOV_EXCL_STOP
