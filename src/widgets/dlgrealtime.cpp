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

#include "core/audioanalyzer.h"
#include "core/audioinput.h"
#include "core/bpmcounter.h"
#include "core/metronome.h"

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

    for (int i = 0; i < NUMDETECTORS; ++i) {
        beatDisplay->addBeatDetector(m_pAnalyzer->beatDetector(i));
    }

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
    m_timer->start(30);
    connect(m_pAnalyzer, SIGNAL(beat(bool)), this, SLOT(slotShowBeat(bool)));

    connect(btnReset, SIGNAL(clicked()), this, SLOT(slotResetCounter()));
    connect(btnBeat, SIGNAL(pressed()), this, SLOT(slotBeat()));
    connect(btnSync, SIGNAL(pressed()), this, SLOT(slotSync()));
}

DlgRealtime::~DlgRealtime() {
    if (m_pInput)
        delete m_pInput;
    if (m_pAnalyzer)
        delete m_pAnalyzer;
    if (m_pCounter)
        delete m_pCounter;
    if (m_pAutoCounter)
        delete m_pAutoCounter;
    if (m_pMetronome)
        delete m_pMetronome;
    delete m_timer;
}

void DlgRealtime::slotUpdateVuMeters(int val) {
    VuLeft->setValue(val);
    VuRight->setValue(val);
}

void DlgRealtime::slotUpdate() {
    VuLeft->setValue(m_pAnalyzer->getVuMeterValueL());
    VuRight->setValue(m_pAnalyzer->getVuMeterValueR());
    magDisplay->setData(m_pAnalyzer->getMagnitude(), m_pAnalyzer->getFFTSize() / 16);
    QString str;

    m_bpmList.append(m_pAnalyzer->getCurrentBPM());
    float bpm = calcMedian();
    float bpmerr = calcError(bpm);

    str = QString::asprintf("%.1f", bpm);
    lcdCurrentBPM->display(str);
    str = QString::asprintf("%.2f", bpmerr);
    lblError->setText(str);
    str = QString::asprintf("%.1f", m_pAnalyzer->getCurrentBPM());
    lblCurrentBPM->setText(str);

    xcorrDisplay->updateData(m_pAnalyzer->getBPMCalculator());
    waveform->update();
    calcWave->update();
    beatDisplay->update();
    magDisplay->update();
    xcorrDisplay->update();
    pbInterval->setValue(m_pMetronome->progressPercent());
}

void DlgRealtime::slotShowBeat(bool beat) {
    button->setChecked(beat);
    if (beat) {
        //m_pAutoCounter->addBeat();
        //qDebug() << "autoCounter:" << m_pAutoCounter->getBPM();
    }
}

void DlgRealtime::slotBeat() {
    m_pCounter->addBeat();
    float bpm = m_pCounter->getBPM();

    QString str;
    str = QString::asprintf("%.2f", bpm);
    lcdTapBPM->display(str);

    str = QString::asprintf("%.2f", m_pCounter->getError());
    lblErrorDisp->setText(str);
    lblBeatsDisp->setText(QString::number(m_pCounter->getBeatCount()));

    if (bpm > 40) {
        m_pMetronome->setBPM(bpm);
        m_pMetronome->setSync();
        m_pMetronome->start();
    } else {
        m_pMetronome->stop();
    }
}

void DlgRealtime::slotResetCounter() {
    m_pCounter->reset();

    lcdTapBPM->display(QStringLiteral("0.00"));
    lblErrorDisp->setText(QStringLiteral("0"));
    lblBeatsDisp->setText(QStringLiteral("0"));
    m_pMetronome->stop();
}

void DlgRealtime::slotResetAutoCounter() {
    m_pAutoCounter->reset();
}

void DlgRealtime::slotSync() {
    m_pMetronome->setSync();
}

float DlgRealtime::calcMedian() {
    const int maxvalues = 100;
    if (m_bpmList.isEmpty())
        return 0;
    // use only last 'maxvalues' values
    while (m_bpmList.size() > maxvalues)
        m_bpmList.removeFirst();

    // make a copy of the list
    QList<float> sortlist = m_bpmList;
    // remove zeros from the list
    sortlist.removeAll(0);
    if (sortlist.isEmpty())
        return 0;

    // sort the list and return the median
    std::sort(sortlist.begin(), sortlist.end());
    int idx = sortlist.size() / 2;
    if (sortlist.size() % 2 == 0) {
        return (sortlist.at(idx) + sortlist.at(idx - 1)) / 2.0;
    } else {
        return sortlist.at(idx);
    }

    return 0;
}

float DlgRealtime::calcError(const float bpm) {
    float error = 0;
    int num = 0;

    for (int i = 0; i < m_bpmList.size(); ++i) {
        float val = m_bpmList.at(i);
        if (val > 0) {
            float cerr = val - bpm;
            ++num;
            error += cerr;
        }
    }
    if (!num)
        return 0;
    return fabs(error / num);
}
