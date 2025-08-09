// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define DEFAULT_THRESHOLD 1.0

class EnergyBeatDetector {
public:
    EnergyBeatDetector(unsigned long bufSize = 10);
    ~EnergyBeatDetector();

    void addValue(float val);
    void setBufferSize(unsigned long bufSize);
    bool isBeat();
    float beat();

    float getAverage() const;
    float getBeatMinimum() const;
    float getCurrentValue() const;
    float getPrevValue() const;
    float getThreshold() const;
    void setThreshold(float val);

protected:
    void updateValues();

private:
    float m_avgEnergy;
    float m_beatMin;
    float m_threshold;
    float m_currentValue;
    float m_prevValue;
    float envelopeAccu;

    unsigned long m_energyBufSize;
    unsigned long m_energyBufIdx; ///< index of new value in energy buffer
    float *m_pEnergyBuffer;       ///< sound energy buffer for last second
};
