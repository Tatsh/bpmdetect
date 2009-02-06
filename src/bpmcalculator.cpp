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

#include <QDebug>

#include "bpmcalculator.h"
#include "PeakFinder.h"

#include <math.h>
#include <cstdlib>
#include <cstring>

#define MIN_BPM 50
#define MAX_BPM 230

BPMCalculator::BPMCalculator(float srate, float length) {
    xcorr = 0;
    m_srate = 0;
    m_length = 0;
    envelopeAccu = 0;
    m_corrMax = 0;
    m_wave.setBufferSize(1);
    setSamplerate(srate);
    setLength(length);
}

BPMCalculator::~BPMCalculator() {
}

void BPMCalculator::setSamplerate(float srate) {
    float oldsrate = m_srate;
    m_srate = fabs(srate);
    if(m_srate != oldsrate) {
        m_windowLen = (60 * m_srate) / MIN_BPM;
        m_windowStart = (60 * m_srate) / MAX_BPM;

        xcorr = (float*) realloc(xcorr, m_windowLen * sizeof(float));
        memset(xcorr, 0, m_windowLen * sizeof(float));
        m_wave.setSamplerate(m_srate);
    }
}

void BPMCalculator::setLength(float seconds) {
    float oldlen = m_length;
    m_length = fabs(seconds);
    if(m_length != oldlen) m_wave.setLength(m_length);
}

Waveform* BPMCalculator::waveform() {
    return &m_wave;
}

const float* BPMCalculator::xcorrData(int& winStart, int& winLen) const {
    winStart = m_windowStart;
    winLen = m_windowLen;
    return xcorr;
}

const std::vector<Peak>& BPMCalculator::peaks() const {
    return m_peaks;
}

void BPMCalculator::calcEnvelope(float* samples, unsigned long numsamples) {
    const float decay = 0.7f;   // decay constant for smoothing the envelope
    const float norm = (1 - decay);

    for (int i = 0; i < numsamples; i ++) {
        float val = fabs(samples[i]);

        // smooth amplitude envelope
        envelopeAccu *= decay;
        envelopeAccu += val;
        val = envelopeAccu * norm;

        samples[i] = val;
    }
}

void BPMCalculator::calcXCorr() {
    const float* pBuffer = m_wave.valueBuffer();
    int numSamples = m_wave.size() - m_windowLen;

    // reset xcorr values
    for(int i = 0; i < m_windowLen; ++i) xcorr[i] = 0;

    for (int offs = m_windowStart; offs < m_windowLen; offs ++) {
        float sum = 0;
        for (int i = 0; i < numSamples; i++) {
            sum += pBuffer[i] * pBuffer[i + offs];
        }

        xcorr[offs] += sum;
    }


    float corrMin = m_corrMax = xcorr[m_windowStart];
    for(int i = m_windowStart+1; i < m_windowLen; ++i) {
        if(xcorr[i] < corrMin) corrMin = xcorr[i];
        if(xcorr[i] > m_corrMax) m_corrMax = xcorr[i];
    }

    for(int i = m_windowStart; i < m_windowLen; ++i) xcorr[i] -= corrMin;
    m_corrMax -= corrMin;
}

void BPMCalculator::findPeaks() {
    float peak, ground, prev, lastGround, lastPeak;
    int dir = 0;
    float minDiff = m_corrMax / 4.;
    if(minDiff < 1000) return;
    lastGround = lastPeak = peak = ground = prev = xcorr[m_windowStart];

    m_peaks.clear();
    Peak p;   p.firstPos = m_windowStart; p.lastPos = m_windowLen; p.peakPos = 0; p.bpm = 0;

    for(int i = m_windowStart+1; i < m_windowLen; ++i) {
        float cur = xcorr[i];
        int prevdir = dir;
        if(cur > prev) {
            prev = peak = cur;
            dir = 1;
        } else if(cur < prev) {
            prev = ground = cur;
            dir = -1;
        }

        if(dir != prevdir && prevdir != 0) {
            if(dir < 0 && peak - lastGround > minDiff) {
                lastPeak = peak;
                p.peakPos = i-1;
            } else if(dir > 0 && lastPeak - ground) {
                lastGround = ground;
                if(p.peakPos) {
                    p.lastPos = i-1;
                    calcMassCenter(p);
                    m_peaks.push_back(p);
                    p.peakPos = 0;
                    p.firstPos = p.lastPos;
                } else p.firstPos = i-1;
            }
        }
    }
}

void BPMCalculator::calcMassCenter(Peak& p) {
    float sum = 0, wsum = 0;

    for(int i = p.firstPos; i <= p.lastPos; i++) {
        sum += (float)i * xcorr[i];
        wsum += xcorr[i];
    }
    p.massCenter = sum / wsum;
    if(p.massCenter > 0) p.bpm = 60.0f * m_srate / p.massCenter;
}

void BPMCalculator::update(float* samples, unsigned long size) {
    calcEnvelope(samples, size);
    m_wave.update(samples, size);
    calcXCorr();
    findPeaks();
}

float BPMCalculator::getBpm() {
/*
    float peakPos = 0;
    if(m_peaks.size()) peakPos = m_peaks[0].massCenter;
    if(peakPos < 1e-6) return 0;
*/
    // calculate BPM
    
    if(m_peaks.size()) return m_peaks[0].bpm;
    //return 60.0f * m_srate / peakPos;
    return 0;
}
