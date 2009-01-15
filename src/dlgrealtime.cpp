/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
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

#include "dlgrealtime.h"
#include <QDebug>

#include "audioinput.h"
#include "audioanalyzer.h"

DlgRealtime::DlgRealtime(QWidget *parent) : QDialog(parent) {
    setupUi(this);
    VuLeft->setValue(0.0);
    VuRight->setValue(10.);

    m_pInput = new AudioInput(44100, 2);
    m_pAnalyzer = new AudioAnalyzer();
    m_pInput->start(m_pAnalyzer);

    waveform->setWaveform(m_pAnalyzer->waveform());
    
    for(int i = 0; i < NUMDETECTORS; ++i) {
        beatDisplay->addBeatDetector(m_pAnalyzer->beatDetector(i));
    }

    connect(m_pAnalyzer, SIGNAL(updated()), this, SLOT(slotUpdate()));
//    connect(&m_qTimer, SIGNAL(timeout()), this, SLOT(slotRefresh()));
//    m_qTimer.start(500);
    connect(m_pAnalyzer, SIGNAL(magnitude(float*, int)), this, SLOT(slotMagnitude(float*, int)));
    connect(m_pAnalyzer, SIGNAL(beat(bool)), this, SLOT(slotShowBeat(bool)));
}


DlgRealtime::~DlgRealtime() {
    if(m_pInput) delete m_pInput;
    if(m_pAnalyzer) delete m_pAnalyzer;
    m_pInput = 0;
    m_pAnalyzer = 0;
}

void DlgRealtime::slotUpdateVuMeters(int val) {
    VuLeft->setValue(val);
    VuRight->setValue(val);
}

void DlgRealtime::slotUpdate() {
    VuLeft->setValue(m_pAnalyzer->getVuMeterValueL());
    VuRight->setValue(m_pAnalyzer->getVuMeterValueR());
    
    waveform->update();
    beatDisplay->update();
}

void DlgRealtime::slotMagnitude(float *mag, int size) {
    magDisplay->setData(mag, size/16);
    magDisplay->update();
}

void DlgRealtime::slotShowBeat(bool beat) {
    button->setChecked(beat);
}
