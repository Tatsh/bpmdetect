/***************************************************************************
     Copyright          : (C) 2009 by Martin Sakmar
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

#include "bpmcounter.h"
#include "beatinfo.h"

#include <QDebug>
#include <QTime>

using namespace std;

BPMCounter::BPMCounter() {
    reset();
    setMinBPM();
    setMaxBPM();
}

BPMCounter::~BPMCounter() {}

/**
 * @brief Correct BPM
 * if dBPM is lower than min or greater than max by multiplying or dividing by 2.
 * if can't find bpm within specified range, returns 0 if blimit is true 
 * or greater bpm than max if blimit is false
 * @return corrected BPM
 */
double BPMCounter::correctBPM(double dBPM, float min, float max, bool blimit, double rBPM) {
    if ( dBPM < 1 ) return 0.;

    // use suggested bpm (rBPM)
    if(rBPM > min && rBPM < max) {
        double diff = 0, lbpm;
        while(dBPM >= rBPM) dBPM /= 2.; // find lower bpm than rBPM
        lbpm = dBPM;
        while(dBPM <= rBPM) dBPM *= 2.; // find greater bpm than rBPM
        if((rBPM - lbpm) < (dBPM - rBPM)) {
            dBPM = lbpm;
        }
        if (blimit && (dBPM < min || dBPM > max)) dBPM = 0;
    } else {
        while(dBPM / 2.0 > min) dBPM /= 2.; // minimum bpm
        while(dBPM * 2.0 < max) dBPM *= 2.; // maximum bpm

        if (dBPM < min || dBPM > max) {
            dBPM = blimit ? 0 : dBPM * 2.;
        }
    }

    return dBPM;
}

void BPMCounter::setMinBPM(unsigned int minBPM) {
    if(minBPM < 40) minBPM = 40;
    if(minBPM > 200) minBPM = 200;
    m_minBPM = minBPM;
}

void BPMCounter::setMaxBPM(unsigned int maxBPM) {
    if(maxBPM > 250) maxBPM = 250;
    if(maxBPM <= m_minBPM+10) return;
    m_maxBPM = maxBPM;
}

void BPMCounter::reset() {
    m_beatCount = -1;
    m_fBPM = m_fError = 0;

    for(int i = 0; i < BUFFER_SIZE; ++i) m_bpmbuffer[i] = 0;
}

void BPMCounter::addBeat() {
    if(m_beatCount < 0) {
        // first beat: start the timer and return
        m_qstarttime = QTime::currentTime();
        m_qtime.start();
        m_beatCount++;
        return;
    }

    // not a first beat
    int mstime = m_qtime.restart();
    float cbpm = 60000.0 / (float)(mstime);
    //qDebug() << "QTime BPM:" << cbpm << "miliseconds:" << mstime;

    // if cbpm is 2 times higher than max bpm, wait for next beat
    if(cbpm > (m_maxBPM*2)) return;

#define GAP_BEATS 5.0
    // reset if gap is longer than GAP_BEATS beats
    if(m_beatCount > 5 && cbpm < m_fBPM / GAP_BEATS) {
        qDebug() << "Long gap, more than " << GAP_BEATS << "beats, resetting";
        reset();
        m_qstarttime = QTime::currentTime();
        m_qtime.start();
        m_beatCount++;
        return;
    }

    float ccbpm = cbpm;
    // FIXME: correct beat count for first 5 values
    const float error = 0.5;
    if(m_beatCount > 5) {
        // calculate average bpm from buffer
        int count = 0; float avgbpm = 0;
        for(int i = 0; i < BUFFER_SIZE; ++i) {
            if(m_bpmbuffer[i] > 1) { avgbpm += m_bpmbuffer[i]; ++count; }
        }
        if(count > 1) avgbpm /= (float) count;
/*
        ccbpm = correctBPM(cbpm, m_minBPM, m_maxBPM, true, avgbpm);
if(ccbpm != cbpm) qDebug() << "BPM corrected (original, corrected):" << cbpm << ccbpm;

        if(m_fBPM > avgbpm * (1.+error)) {
            qDebug() << "current bpm too high, resetting ...";
            reset();
            return;
        } else if(m_fBPM < avgbpm * (1.-error)) {
            qDebug() << "current bpm too low, resetting ...";
            reset();
            return;
        }
*/
    }

    // shift bpm buffer and add new value
    for(int i = 1; i < BUFFER_SIZE; ++i) m_bpmbuffer[i-1] = m_bpmbuffer[i];
    m_bpmbuffer[BUFFER_SIZE-1] = ccbpm;

    int beats = ccbpm / cbpm;
    m_beatCount += beats;
    calcBPM();
}

float BPMCounter::getBPM() const {
    return m_fBPM;
}

float BPMCounter::getError() const {
    return m_fError;
}

long BPMCounter::getBeatCount() const {
    if(m_beatCount < 0) return 0;
    return m_beatCount;
}

void BPMCounter::calcBPM() {
    if(m_beatCount < 1) {
        m_fBPM = m_fError = 0;
        return;
    }

    int timems = m_qstarttime.msecsTo(QTime::currentTime());
    m_fBPM = (float)m_beatCount * 60000.0 / (float)(timems);
    m_fError = (60000.0 / (float)(timems)) / m_fBPM * 100.;
}
