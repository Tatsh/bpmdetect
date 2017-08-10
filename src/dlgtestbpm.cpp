/***************************************************************************
     Copyright          : (C) 2007 by Martin Sakmar
     e-mail             : martin.sakmar@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QString>
#include <QComboBox>
#include <QLabel>
#include <QDebug>

#include "dlgtestbpm.h"
#include "dlgtestbpmplayer.h"
#include "progressbar.h"
#include "track.h"

using namespace std;

DlgTestBPM::DlgTestBPM(const QString file, const float bpm, QWidget *parent) : QDialog( parent ) {
    setupUi(this);

    if (file.isEmpty()) {
        close();
    }

    player = new DlgTestBPMPlayer(file, cbNBeats->currentText().toInt(), bpm, 0, this);
    m_bpm = bpm;

    lblBPM->setText(QString::fromStdString(Track::bpm2str(bpm, "000.00")));
    connect(trackPosition, SIGNAL(positionChanged(uint)), this, SLOT(setCustomPos( uint )));
    connect(player, SIGNAL(hasLengthUS(const qint64)), this, SLOT(setTrackPositionLength(const qint64)));

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

void DlgTestBPM::setTrackPositionLength(const qint64 length) {
    trackPosition->setLength(length / 1000);
    cbNBeats->setEnabled(true);
    btnPos1->setEnabled(true);
    btnPos2->setEnabled(true);
    btnPos3->setEnabled(true);
    btnPos4->setEnabled(true);
    trackPosition->setEnabled(true);
}

void DlgTestBPM::setPos1() {
    uint msec = trackPosition->length() / 5;
    player->update(cbNBeats->currentText().toInt(), player->getLengthUS() * 0.2);
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setPos2() {
    uint msec = ( trackPosition->length() * 2 ) / 5;
    player->update(cbNBeats->currentText().toInt(), player->getLengthUS() * 0.4);
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setPos3() {
    uint msec = ( trackPosition->length() * 3 ) / 5;
    player->update(cbNBeats->currentText().toInt(), player->getLengthUS() * 0.6);
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setPos4() {
    uint msec = ( trackPosition->length() * 4 ) / 5;
    player->update(cbNBeats->currentText().toInt(), player->getLengthUS() * 0.8);
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setCustomPos( uint msec ) {
    player->update(cbNBeats->currentText().toInt(), trackPosition->value() * 1000);
}

/**
 * @brief Set number of beats to loop
 * called when combobox currentItemChanged is emited
 * @param text is number of beats to loop (combobox currentText)
 */
void DlgTestBPM::setNumBeats( const QString &text ) {
    int value = trackPosition->value();
    if (value < 0) {
        value = 0;
    }
    player->update(cbNBeats->currentText().toInt(), value * 1000);
}

void DlgTestBPM::slotUpdateBpmList() {
    const int MIN_BPM = 45;
    const int MAX_BPM = 220;

    m_bpmList.clear();

    if(m_bpm > MIN_BPM && m_bpm < MAX_BPM) {
        float cbpm = m_bpm;
        while(cbpm / 2. > MIN_BPM) cbpm /= 2.;

        const float d = 0.25 * cbpm;
        while(cbpm-d > MIN_BPM) cbpm -= d;

        for(; cbpm < MAX_BPM; cbpm += d) {
            m_bpmList.append(cbpm);
        }
    }
#ifdef DEBUG
    qDebug() << m_bpmList;
    qDebug() << "total list size:" << m_bpmList.size();
#endif
}
