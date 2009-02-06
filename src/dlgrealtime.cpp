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
#include "bpmcounter.h"
#include "metronome.h"

DlgRealtime::DlgRealtime(QWidget *parent) : QDialog(parent) {
    setupUi(this);
    VuLeft->setValue(0.0);
    VuRight->setValue(10.);

    m_pInput = new AudioInput(44100, 2);
    m_pAnalyzer = new AudioAnalyzer();
    m_pCounter = new BPMCounter();
    m_pAutoCounter = new BPMCounter();
    m_pMetronome = new Metronome();

    m_pInput->start(m_pAnalyzer);
    waveform->setWaveform(m_pAnalyzer->waveform());
    calcWave->setAutoScale(true);
    calcWave->setWaveform(m_pAnalyzer->calculatorWave());

    magDisplay->setMaxValue(60);

    for(int i = 0; i < NUMDETECTORS; ++i) {
        beatDisplay->addBeatDetector(m_pAnalyzer->beatDetector(i));
    }

    connect(&m_qTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    m_qTimer.start(20);

    connect(m_pAnalyzer, SIGNAL(updated()), this, SLOT(slotUpdate()));
    connect(m_pAnalyzer, SIGNAL(beat(bool)), this, SLOT(slotShowBeat(bool)));
    
    connect(btnReset, SIGNAL(clicked()), this, SLOT(slotResetCounter()));
    connect(btnBeat, SIGNAL(pressed()), this, SLOT(slotBeat()));
    connect(btnAutoReset, SIGNAL(clicked()), this, SLOT(slotResetAutoCounter()));
}


DlgRealtime::~DlgRealtime() {
    if(m_pInput) delete m_pInput;
    if(m_pAnalyzer) delete m_pAnalyzer;
    if(m_pCounter) delete m_pCounter;
    if(m_pAutoCounter) delete m_pAutoCounter;
    if(m_pMetronome) delete m_pMetronome;
}

void DlgRealtime::slotUpdateVuMeters(int val) {
    VuLeft->setValue(val);
    VuRight->setValue(val);
}

void DlgRealtime::slotUpdate() {
    VuLeft->setValue(m_pAnalyzer->getVuMeterValueL());
    VuRight->setValue(m_pAnalyzer->getVuMeterValueR());
    magDisplay->setData(m_pAnalyzer->getMagnitude(), m_pAnalyzer->getFFTSize()/16);
    lcdCurrentBPM->display(m_pAnalyzer->getCurrentBPM());
    xcorrDisplay->updateData(m_pAnalyzer->getBPMCalculator());

    waveform->update();
    calcWave->update();
    beatDisplay->update();
    magDisplay->update();
    xcorrDisplay->update();
}

void DlgRealtime::slotShowBeat(bool beat) {
    button->setChecked(beat);
    if(beat) {
        m_pAutoCounter->addBeat();
        lcdAverageBPM->display(m_pAutoCounter->getBPM());
    }
}

void DlgRealtime::slotBeat() {
    m_pCounter->addBeat();
    float bpm = m_pCounter->getBPM();
    lcdTapBPM->display(bpm);
    lcdTapError->display(m_pCounter->getError());

    if(bpm > 40) {
        m_pMetronome->setBPM(bpm);
        m_pMetronome->setSync();
        m_pMetronome->start();
    }
}

void DlgRealtime::slotResetCounter() {
    m_pCounter->reset();
    
    lcdTapBPM->display(m_pCounter->getBPM());
    lcdTapError->display(m_pCounter->getError());
    m_pMetronome->stop();
}

void DlgRealtime::slotResetAutoCounter() {
    m_pAutoCounter->reset();
    lcdAverageBPM->display(0);
}

void DlgRealtime::slotTimeout() {
    pbInterval->setValue(m_pMetronome->progressPercent());
}

