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

#ifndef BPMCALCULATOR_H
#define BPMCALCULATOR_H

#include "energybeatdetector.h"
#include "waveform.h"
#include <vector>

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
    void update(float* samples, unsigned long size);
    float getBpm();
    Waveform* waveform();
    const float* xcorrData(int& winStart, int& winLen) const;
    const std::vector<Peak>& peaks() const;
    float lastWaveformValue() const;
    bool isBeat();
    const EnergyBeatDetector& beatDetector() const;

protected:
    void calcEnvelope(float* samples, unsigned long numsamples);
    void calcXCorr();
    void findPeaks();
    void calcMassCenter(Peak& p);

private:
    float m_srate;  ///< input sample rate
    float m_length; ///< length in seconds

    float envelopeAccu;
    int m_windowStart;
    int m_windowLen;
    
    Waveform m_wave, m_waveOrig;
    EnergyBeatDetector m_energyBeat;
    float* xcorr;
    float m_corrMax; /// maximum xcorr value (minimum is 0)
    std::vector<Peak> m_peaks;
};

#endif
