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

#include "energybeatdetector.h"

#include <cstdlib>
#include <cstring>
#include <math.h>

typedef unsigned long ulong;

EnergyBeatDetector::EnergyBeatDetector(unsigned long bufsize) {
    m_avgEnergy = 0;
    m_beatMin = 0;
    m_threshold = 1;
    m_pEnergyBuffer = 0;
    m_currentValue = m_prevValue = 0;

    setBufferSize(bufsize);
}

EnergyBeatDetector::~EnergyBeatDetector() {
    if(m_pEnergyBuffer) delete [] m_pEnergyBuffer;
}

bool EnergyBeatDetector::addValue(float val) {
    m_pEnergyBuffer[m_energyBufIdx++] = val;
    m_prevValue = m_currentValue;
    m_currentValue = val;
    if(m_energyBufIdx >= m_energyBufSize) m_energyBufIdx = 0;
    updateValues();
}

void EnergyBeatDetector::setBufferSize(unsigned long bufsize) {
    if(bufsize < 2) bufsize = 2;
    m_energyBufSize = bufsize;
    m_energyBufIdx = 0;
    m_pEnergyBuffer = (float*) realloc(m_pEnergyBuffer, bufsize * sizeof(float));
    memset(m_pEnergyBuffer, 0, sizeof(float) * m_energyBufSize);
}

bool EnergyBeatDetector::isBeat() {
    if(m_currentValue < 1.5*m_threshold) return false;

    if(m_currentValue > m_beatMin) return true;
    return false;
}

float EnergyBeatDetector::beat() {
    if(m_currentValue <= m_threshold) return 0;
    float fbeat = m_currentValue - m_beatMin;
    return fbeat;
}

float EnergyBeatDetector::getAverage() const {
    return m_avgEnergy;
}

float EnergyBeatDetector::getBeatMinimum() const {
    return m_beatMin;
}

float EnergyBeatDetector::getCurrentValue() const {
    return m_currentValue;
}

float EnergyBeatDetector::getPrevValue() const {
    return m_prevValue;
}

float EnergyBeatDetector::getThreshold() const {
    return m_threshold;
}

void EnergyBeatDetector::setThreshold(float val) {
    m_threshold = val;
}

void EnergyBeatDetector::updateValues() {
    // calculate average energy
    m_avgEnergy = 0;
    for(ulong i = 0; i < m_energyBufSize; ++i) {
        m_avgEnergy += m_pEnergyBuffer[i];
    }
    m_avgEnergy /= m_energyBufSize;

    float variance = 0;
    for(ulong i = 0; i < m_energyBufSize; ++i) {
        variance += powf(m_pEnergyBuffer[i] - m_avgEnergy, 2.0);
    }
    variance /= m_energyBufSize;
    float constant = (-0.0025 * variance) + 1.52;
    if(m_avgEnergy < 3*m_threshold) constant = (-0.0025 * variance) + 1.68;
    if(m_avgEnergy < 2*m_threshold) constant = (-0.0025 * variance) + 1.8;

    m_beatMin = constant * m_avgEnergy;
    if(m_beatMin < m_avgEnergy) m_beatMin = m_avgEnergy;
}

