// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <vector>

#include "energybeatdetector.h"
#include "waveform.h"

struct Peak {
    int firstPos;
    int lastPos;
    int peakPos;
    float massCenter;
    float corrPos;
    float bpm;
    float corrbpm;
};

class BPMCalculator {
public:
    /// @a srate is input sample rate
    /// @a length is internal buffer length in seconds
    BPMCalculator(float srate, float length = 5);
    ~BPMCalculator();

    void setSamplerate(float srate);
    void setLength(float seconds);
    void update(float *samples, unsigned long size);
    float getBpm();
    Waveform *waveform();
    const float *xcorrData(int &winStart, int &winLen) const;
    const std::vector<Peak> &peaks() const;
    float lastWaveformValue() const;
    bool isBeat();
    const EnergyBeatDetector &beatDetector() const;

protected:
    void calcEnvelope(float *samples, unsigned long numsamples);
    void calcXCorr();
    void findPeaks();
    void calcMassCenter(Peak &p);

private:
    float m_srate;  ///< input sample rate
    float m_length; ///< length in seconds

    float envelopeAccu;
    int m_windowStart;
    int m_windowLen;

    Waveform m_wave, m_waveOrig;
    EnergyBeatDetector m_energyBeat;
    float *xcorr;
    float m_corrMax; /// maximum xcorr value (minimum is 0)
    std::vector<Peak> m_peaks;
};
