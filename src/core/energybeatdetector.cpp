// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "energybeatdetector.h"

typedef unsigned long ulong;

EnergyBeatDetector::EnergyBeatDetector(unsigned long bufsize) {
    m_avgEnergy = 0;
    m_beatMin = 0;
    m_threshold = 1;
    m_pEnergyBuffer = nullptr;
    m_currentValue = m_prevValue = 0;
    envelopeAccu = 0;

    setBufferSize(bufsize);
}

EnergyBeatDetector::~EnergyBeatDetector() {
    if (m_pEnergyBuffer)
        delete[] m_pEnergyBuffer;
}

void EnergyBeatDetector::addValue(float val) {
    const float decay = 0.1f;
    const float norm = (1 - decay);
    // smooth amplitude envelope
    envelopeAccu *= decay;
    envelopeAccu += val;
    val = envelopeAccu * norm;

    m_pEnergyBuffer[m_energyBufIdx++] = val;
    m_prevValue = m_currentValue;
    m_currentValue = val;
    if (m_energyBufIdx >= m_energyBufSize)
        m_energyBufIdx = 0;
    updateValues();
}

void EnergyBeatDetector::setBufferSize(unsigned long bufsize) {
    if (bufsize < 2)
        bufsize = 2;
    m_energyBufSize = bufsize;
    m_energyBufIdx = 0;
    m_pEnergyBuffer = static_cast<float *>(realloc(m_pEnergyBuffer, bufsize * sizeof(float)));
    memset(m_pEnergyBuffer, 0, sizeof(float) * m_energyBufSize);
}

bool EnergyBeatDetector::isBeat() {
    if (m_currentValue < 1.5f * m_threshold)
        return false;

    if (m_currentValue > m_beatMin)
        return true;
    return false;
}

float EnergyBeatDetector::beat() {
    if (m_currentValue <= m_threshold)
        return 0;
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
    for (ulong i = 0; i < m_energyBufSize; ++i) {
        m_avgEnergy += m_pEnergyBuffer[i];
    }
    m_avgEnergy /= static_cast<float>(m_energyBufSize);

    float variance = 0;
    for (ulong i = 0; i < m_energyBufSize; ++i) {
        variance += powf(m_pEnergyBuffer[i] - m_avgEnergy, 2.0f);
    }
    variance /= static_cast<float>(m_energyBufSize);
    float constant = (-0.0025f * variance) + 1.52f;
    if (m_avgEnergy < 3 * m_threshold)
        constant = (-0.0025f * variance) + 1.68f;
    if (m_avgEnergy < 2 * m_threshold)
        constant = (-0.0025f * variance) + 1.8f;

    m_beatMin = constant * m_avgEnergy;
    if (m_beatMin < m_avgEnergy)
        m_beatMin = m_avgEnergy;
}
