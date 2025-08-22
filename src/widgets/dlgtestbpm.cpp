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
    : m_player(player), m_bpm(bpm), QDialog(parent) {
    setupUi(this);

    if (file.isEmpty()) {
        // LCOV_EXCL_START
        close();
        // LCOV_EXCL_STOP
    }

    lblBpm->setText(bpmToString(static_cast<bpmtype>(bpm), QStringLiteral("000.00")));
    connect(trackPosition, &ProgressBar::positionChanged, this, &DlgTestBpm::setCustomPos);
    connect(player, &DlgTestBpmPlayer::hasLengthUS, this, &DlgTestBpm::setTrackPositionLength);
    connect(player, &DlgTestBpmPlayer::audioError, [this](QAudio::Error e) {
        // LCOV_EXCL_START
        if (e == QAudio::FatalError) {
            qCCritical(gLogBpmDetect) << "Fatal audio error occurred.";
            reject();
        }
    });
    // LCOV_EXCL_STOP
    connect(this, &QDialog::accepted, player, &DlgTestBpmPlayer::stop);
    connect(this, &QDialog::rejected, player, &DlgTestBpmPlayer::stop);
    connect(this->cbNBeats, &QComboBox::currentTextChanged, this, &DlgTestBpm::setNumBeats);
    connect(this->btnPos1, &QPushButton::clicked, [this]() { setPosFromButton(1); });
    connect(this->btnPos2, &QPushButton::clicked, [this]() { setPosFromButton(2); });
    connect(this->btnPos3, &QPushButton::clicked, [this]() { setPosFromButton(3); });
    connect(this->btnPos4, &QPushButton::clicked, [this]() { setPosFromButton(4); });

    cbNBeats->setEnabled(false);
    btnPos1->setEnabled(false);
    btnPos2->setEnabled(false);
    btnPos3->setEnabled(false);
    btnPos4->setEnabled(false);
    trackPosition->setEnabled(false);

    slotUpdateBpmList();
#ifndef TESTING
    m_player->start();
#endif
}

DlgTestBpm::~DlgTestBpm() {
    if (m_player) {
        m_player->stop();
    }
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
    m_player->update(
        cbNBeats->currentText().toUInt(),
        static_cast<qint64>(static_cast<double>(m_player->lengthUs()) * (index * 0.2)));
    trackPosition->setPosition((trackPosition->length() * index) / 5);
}

void DlgTestBpm::setCustomPos(int msec) {
    Q_UNUSED(msec)
    m_player->update(cbNBeats->currentText().toUInt(), trackPosition->value() * 1000);
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
    m_player->update(cbNBeats->currentText().toUInt(), value * 1000);
}

void DlgTestBpm::slotUpdateBpmList() {
    const auto MIN_BPM = 45;
    const auto MAX_BPM = 220;

    m_bpmList.clear();

    if (m_bpm > MIN_BPM && m_bpm < MAX_BPM) {
        auto cBpm = m_bpm;
        while (cBpm / 2.f > MIN_BPM)
            cBpm /= 2.f;

        const auto d = 0.25f * cBpm;
        while (cBpm - d > MIN_BPM)
            cBpm -= d;

        for (; cBpm < MAX_BPM; cBpm += d) {
            m_bpmList.append(cBpm);
        }
    }
}
