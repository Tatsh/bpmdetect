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

#include <sys/time.h>

#include <QDebug>

using namespace std;

BPMCounter::BPMCounter() {
    reset();
    setMinBPM();
    setMaxBPM();
}

BPMCounter::~BPMCounter() {}

/**
 * @brief Correct BPM
 * if value is lower than min or greater than max
 * @return corrected BPM
 */
double BPMCounter::correctBPM(double dBPM, float min, float max, bool blimit) {
    if ( dBPM < 1 ) return 0.;
    while ( dBPM > max ) dBPM /= 2.;
    while ( dBPM < min ) dBPM *= 2.;

    if (blimit && dBPM > max) {
        dBPM = 0.;
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
    m_tstart = m_tstart = m_tprev = m_ntstart = m_beatCount = m_nbeatCount = 0;
    m_fBPM = m_fError = 0;
    
    for(int i = 0; i < BUFFER_SIZE; ++i) m_bpmbuffer[i] = 0;
}

void BPMCounter::addBeat() {
    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec*1000 + time.tv_usec/1000;

    if(!m_tstart) {
        m_tstart = m_tcurr = msec;
        m_beatCount = 0;
        return;
    }

    m_tprev = m_tcurr;
    m_tcurr = msec;
    float cbpm = 60000.0 / (float)(m_tcurr - m_tprev);

    // if cbpm is too high, wait for next beat
    if(cbpm > (m_maxBPM*2.)) return;

#define GAP_BEATS 5.0
    // reset if gap is longer than GAP_BEATS beats
    if(m_beatCount > 5 && cbpm < m_fBPM / GAP_BEATS) {
        reset();
        m_tstart = msec;
        return;
    }

    // now correct the cbpm value so it will be between min and max BPMCounter
    float ccbpm = correctBPM(cbpm, m_minBPM, m_maxBPM);

    // calculate average bpm from buffer
    int count = 0; float avgbpm = 0;
    for(int i = 0; i < BUFFER_SIZE; ++i) {
        if(m_bpmbuffer[i] > 1) { avgbpm += m_bpmbuffer[i]; ++count; }
    }
    if(count > 1) avgbpm /= (float) count;

qDebug() << "current bpm: " << ccbpm << cbpm << "average:" << avgbpm;

    // TODO: calculate current error, store start time
    // add bpm to new average buffer and store new beat count
    // if new beats count is at least 5 switch current values with new
    // (tstart, beatcount, bpmbuffer etc)
    
    const float error = 0.4;
    if(m_beatCount > 5) {
        if(ccbpm > (1.+error) * m_fBPM) {
            qDebug() << "TODO: ccbpm too high, resetting ...";
            //m_tstart = m_tcurr;
            //m_beatCount = 0;
            reset();
            return;
        } else if(ccbpm < (1.-error) * m_fBPM) {
            qDebug() << "TODO: ccbpm too low, resetting ...";
            //m_tstart = m_tcurr;
            //m_beatCount = 0;
            reset();
            return;
        }
    }
    
    // add ccbpm to avgbpmbuf
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

unsigned long BPMCounter::getBeatCount() const {
    return m_beatCount;
}

void BPMCounter::calcBPM() {
    if(m_beatCount < 1) {
        m_fBPM = m_fError = 0;
        return;
    }

    m_fBPM = (float)m_beatCount * 60000.0 / (float)(m_tcurr - m_tstart);
    m_fError = (60000.0 / (float)(m_tcurr - m_tstart)) / m_fBPM * 100.;
}
