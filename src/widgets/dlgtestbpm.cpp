// SPDX-License-Identifier: GPL-3.0-or-later
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QString>

#include "dlgtestbpm.h"
#include "dlgtestbpmplayer.h"
#include "progressbar.h"
#include "track/track.h"

using namespace std;

DlgTestBPM::DlgTestBPM(const QString file, const float bpm, QWidget *parent) : QDialog(parent) {
    setupUi(this);

    if (file.isEmpty()) {
        close();
    }

    player = new DlgTestBPMPlayer(
        file, static_cast<uint>(cbNBeats->currentText().toInt()), static_cast<uint>(bpm), 0, this);
    m_bpm = bpm;

    lblBPM->setText(Track::bpm2str(static_cast<double>(bpm), QStringLiteral("000.00")));
    connect(trackPosition, SIGNAL(positionChanged(qint64)), this, SLOT(setCustomPos(qint64)));
    connect(player, SIGNAL(hasLengthUS(qint64)), this, SLOT(setTrackPositionLength(qint64)));
    connect(this, &QDialog::accepted, player, &DlgTestBPMPlayer::stop);
    connect(this, &QDialog::rejected, player, &DlgTestBPMPlayer::stop);
    connect(this->cbNBeats, &QComboBox::currentTextChanged, this, &DlgTestBPM::setNumBeats);

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
    trackPosition->setLength(static_cast<uint>(length / 1000));
    cbNBeats->setEnabled(true);
    btnPos1->setEnabled(true);
    btnPos2->setEnabled(true);
    btnPos3->setEnabled(true);
    btnPos4->setEnabled(true);
    trackPosition->setEnabled(true);
}

void DlgTestBPM::setPos1() {
    uint msec = trackPosition->length() / 5;
    player->update(cbNBeats->currentText().toUInt(),
                   static_cast<qint64>(static_cast<double>(player->getLengthUS()) * 0.2));
    trackPosition->setPosition(msec);
}

void DlgTestBPM::setPos2() {
    uint msec = (trackPosition->length() * 2) / 5;
    player->update(cbNBeats->currentText().toUInt(),
                   static_cast<qint64>(static_cast<double>(player->getLengthUS()) * 0.4));
    trackPosition->setPosition(msec);
}

void DlgTestBPM::setPos3() {
    uint msec = (trackPosition->length() * 3) / 5;
    player->update(cbNBeats->currentText().toUInt(),
                   static_cast<qint64>(static_cast<double>(player->getLengthUS()) * 0.6));
    trackPosition->setPosition(msec);
}

void DlgTestBPM::setPos4() {
    uint msec = (trackPosition->length() * 4) / 5;
    player->update(cbNBeats->currentText().toUInt(),
                   static_cast<qint64>(static_cast<double>(player->getLengthUS()) * 0.8));
    trackPosition->setPosition(msec);
}

void DlgTestBPM::setCustomPos(qint64 msec) {
    Q_UNUSED(msec)
    player->update(cbNBeats->currentText().toUInt(), trackPosition->value() * 1000);
}

void DlgTestBPM::setNumBeats(const QString &s) {
    int value = trackPosition->value();
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
    const int MIN_BPM = 45;
    const int MAX_BPM = 220;

    m_bpmList.clear();

    if (m_bpm > MIN_BPM && m_bpm < MAX_BPM) {
        float cbpm = m_bpm;
        while (cbpm / 2.f > MIN_BPM)
            cbpm /= 2.f;

        const float d = 0.25f * cbpm;
        while (cbpm - d > MIN_BPM)
            cbpm -= d;

        for (; cbpm < MAX_BPM; cbpm += d) {
            m_bpmList.append(cbpm);
        }
    }
#ifdef DEBUG
    qDebug() << m_bpmList;
    qDebug() << "total list size:" << m_bpmList.size();
#endif
}
