// SPDX-License-Identifier: GPL-3.0-or-later
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QString>

#include "dlgtestbpm.h"
#include "dlgtestbpmplayer.h"
#include "progressbar.h"
#include "track/track.h"

DlgTestBPM::DlgTestBPM(const QString file, const float bpm, QWidget *parent) : QDialog(parent) {
    setupUi(this);

    if (file.isEmpty()) {
        close();
    }

    player = new DlgTestBPMPlayer(
        file, static_cast<uint>(cbNBeats->currentText().toInt()), static_cast<uint>(bpm), 0, this);
    m_bpm = bpm;

    lblBPM->setText(Track::bpm2str(static_cast<double>(bpm), QStringLiteral("000.00")));
    connect(trackPosition, &ProgressBar::positionChanged, this, &DlgTestBPM::setCustomPos);
    connect(player, &DlgTestBPMPlayer::hasLengthUS, this, &DlgTestBPM::setTrackPositionLength);
    connect(player, &DlgTestBPMPlayer::audioError, [this](QAudio::Error e) {
        if (e == QAudio::FatalError) {
            qCritical() << "Fatal audio error occurred.";
            reject();
        }
    });
    connect(this, &QDialog::accepted, player, &DlgTestBPMPlayer::stop);
    connect(this, &QDialog::rejected, player, &DlgTestBPMPlayer::stop);
    connect(this->cbNBeats, &QComboBox::currentTextChanged, this, &DlgTestBPM::setNumBeats);
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
    player->start();
}

DlgTestBPM::~DlgTestBPM() {
    player->stop();
}

void DlgTestBPM::setTrackPositionLength(qint64 length) {
    trackPosition->setLength(static_cast<int>(length) / 1000);
    cbNBeats->setEnabled(true);
    btnPos1->setEnabled(true);
    btnPos2->setEnabled(true);
    btnPos3->setEnabled(true);
    btnPos4->setEnabled(true);
    trackPosition->setEnabled(true);
}

void DlgTestBPM::setPosFromButton(int index) {
    auto msec = (trackPosition->length() * index) / 5;
    player->update(cbNBeats->currentText().toUInt(),
                   static_cast<qint64>(static_cast<double>(player->getLengthUS()) * (index * 0.2)));
    trackPosition->setPosition(msec);
}

void DlgTestBPM::setCustomPos(int msec) {
    Q_UNUSED(msec)
    player->update(cbNBeats->currentText().toUInt(), trackPosition->value() * 1000);
}

void DlgTestBPM::setNumBeats(const QString &s) {
    auto value = trackPosition->value();
    if (value < 0) {
        value = 0;
    }
    const auto num = s.toInt();
    if (num <= 0) {
        return;
    }
    player->update(cbNBeats->currentText().toUInt(), value * 1000);
}

void DlgTestBPM::slotUpdateBpmList() {
    const auto MIN_BPM = 45;
    const auto MAX_BPM = 220;

    m_bpmList.clear();

    if (m_bpm > MIN_BPM && m_bpm < MAX_BPM) {
        auto cBPM = m_bpm;
        while (cBPM / 2.f > MIN_BPM)
            cBPM /= 2.f;

        const auto d = 0.25f * cBPM;
        while (cBPM - d > MIN_BPM)
            cBPM -= d;

        for (; cBPM < MAX_BPM; cBPM += d) {
            m_bpmList.append(cBPM);
        }
    }
#ifndef NDEBUG
    qDebug() << m_bpmList;
    qDebug() << "total list size:" << m_bpmList.size();
#endif
}
