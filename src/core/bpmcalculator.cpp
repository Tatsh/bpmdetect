// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <QDebug>

#include "PeakFinder.h"
#include "bpmcalculator.h"

#define MIN_BPM 50
#define MAX_BPM 230
#define MIN_THRESHOLD 0.1

/// decay constant for calculating RMS volume sliding average approximation
static const float avgdecay = 0.99987f;
/// Normalization coefficient for calculating RMS sliding average approximation.
static const float avgnorm = (1 - avgdecay);

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
    if (m_srate != oldsrate) {
        m_windowLen = (60 * m_srate) / MIN_BPM;
        m_windowStart = (60 * m_srate) / MAX_BPM;

        xcorr = (float *)realloc(xcorr, m_windowLen * sizeof(float));
        memset(xcorr, 0, m_windowLen * sizeof(float));
        m_wave.setSamplerate(m_srate);
        m_waveOrig.setSamplerate(m_srate);
        m_energyBeat.setBufferSize(m_srate);
    }
}

void BPMCalculator::setLength(float seconds) {
    float oldlen = m_length;
    m_length = fabs(seconds);
    if (m_length != oldlen) {
        m_wave.setLength(m_length);
        m_waveOrig.setLength(m_length);
    }
}

Waveform *BPMCalculator::waveform() {
    return &m_wave;
}

float BPMCalculator::lastWaveformValue() const {
    return m_wave.valueBuffer()[m_wave.size() - 1];
}

const float *BPMCalculator::xcorrData(int &winStart, int &winLen) const {
    winStart = m_windowStart;
    winLen = m_windowLen;
    return xcorr;
}

const std::vector<Peak> &BPMCalculator::peaks() const {
    return m_peaks;
}

void BPMCalculator::calcEnvelope(float *samples, unsigned long numsamples) {
    const float decay = 0.7f; // decay constant for smoothing the envelope
    const float norm = (1 - decay);

    for (unsigned long i = 0; i < numsamples; i++) {
        float val = fabs(samples[i]);

        float threshold = (m_waveOrig.getAverageValue() - m_waveOrig.getMinValue()) / 2.0;
        if (threshold < MIN_THRESHOLD)
            threshold = MIN_THRESHOLD;

        // cut amplitudes (we're interested in peak values, not the silent moments)
        val -= threshold;
        val = (val > 0) ? val : 0;

        //qDebug() << "avg, val, max" << avgval << val << m_waveOrig.getMaxValue();

        // smooth amplitude envelope
        envelopeAccu *= decay;
        envelopeAccu += val;
        val = envelopeAccu * norm;
        //qDebug() << "val" << val;
        samples[i] = val;
    }
}

void BPMCalculator::calcXCorr() {
    const float *pBuffer = m_wave.valueBuffer();
    int numSamples = m_wave.size() - m_windowLen;

    // reset xcorr values
    for (int i = 0; i < m_windowLen; ++i)
        xcorr[i] = 0;

    for (int offs = m_windowStart; offs < m_windowLen; offs++) {
        float sum = 0;
        for (int i = 0; i < numSamples; i++) {
            sum += pBuffer[i] * pBuffer[i + offs];
        }

        xcorr[offs] += sum;
    }

    float corrMin = m_corrMax = xcorr[m_windowStart];
    for (int i = m_windowStart + 1; i < m_windowLen; ++i) {
        if (xcorr[i] < corrMin)
            corrMin = xcorr[i];
        if (xcorr[i] > m_corrMax)
            m_corrMax = xcorr[i];
    }

    for (int i = m_windowStart; i < m_windowLen; ++i)
        xcorr[i] -= corrMin;
    m_corrMax -= corrMin;
}

void BPMCalculator::findPeaks() {
    float peak, ground, prev;
    int dir = 0;
    float minDiff = m_corrMax / 5.;
    if (minDiff < 0.02)
        return;
    peak = ground = prev = xcorr[m_windowStart];

    m_peaks.clear();
    Peak p;
    p.firstPos = m_windowStart;
    p.lastPos = m_windowLen;
    p.peakPos = 0;
    p.bpm = 0;

    for (int i = m_windowStart + 1; i < m_windowLen; ++i) {
        float cur = xcorr[i];
        int prevdir = dir;
        if (cur > prev) {
            prev = peak = cur;
            dir = 1;
        } else if (cur < prev) {
            prev = ground = cur;
            dir = -1;
        }

        if (dir != prevdir && prevdir != 0) {
            if (dir < 0 /*&& peak - lastGround > minDiff*/) {
                // lastPeak = peak;
                p.peakPos = i - 1;
            } else if (dir > 0 /*&& lastPeak - ground > minDiff*/) {
                // lastGround = ground;
                if (p.peakPos) {
                    p.lastPos = i - 1;
                    calcMassCenter(p);
                    if (xcorr[p.peakPos] - xcorr[p.firstPos] > minDiff &&
                        xcorr[p.peakPos] - xcorr[p.lastPos] > minDiff) {
                        m_peaks.push_back(p);
                    }
                    p.peakPos = 0;
                    p.firstPos = p.lastPos;
                } else
                    p.firstPos = i - 1;
            }
        }
    }
}

void BPMCalculator::calcMassCenter(Peak &p) {
    float sum = 0, wsum = 0;

    for (int i = p.firstPos; i <= p.lastPos; i++) {
        sum += (float)i * xcorr[i];
        wsum += xcorr[i];
    }
    p.massCenter = sum / wsum;
    if (p.massCenter > 0)
        p.bpm = 60.0f * m_srate / p.massCenter;
    p.corrbpm = p.bpm;
}

void BPMCalculator::update(float *samples, unsigned long size) {
    m_waveOrig.update(samples, size);
    calcEnvelope(samples, size);
    m_wave.update(samples, size);
    calcXCorr();
    findPeaks();

    for (unsigned long i = 0; i < size; ++i) {
        m_energyBeat.addValue(samples[i]);
    }
}

const EnergyBeatDetector &BPMCalculator::beatDetector() const {
    return m_energyBeat;
}

bool BPMCalculator::isBeat() {
    return m_energyBeat.isBeat();
}

float BPMCalculator::getBpm() {
    /*
    float peakPos = 0;
    if(m_peaks.size()) peakPos = m_peaks[0].massCenter;
    if(peakPos < 1e-6) return 0;
*/

    if (m_peaks.size() < 1)
        return 0;

    // calculate BPM
    float bpm[m_peaks.size()]; // bpms for every peak
    int bpmn[m_peaks.size()];  // number of similar bpms
    for (std::size_t i = 0; i < m_peaks.size(); ++i) {
        bpm[i] = m_peaks[i].bpm;
        bpmn[i] = 1;
    }

    for (int i = m_peaks.size() - 1; i; i--) {
        float bpm2 = bpm[i] * 2;
        for (std::size_t j = 0; j < m_peaks.size(); j++) {
            if (bpm[j] + 3 > bpm2 && bpm[j] - 3 < bpm2) {
                bpm[i] = bpm2;
                bpmn[i]++;
                m_peaks[i].corrbpm = bpm2;
                m_peaks[i].corrPos = 60.0 * m_srate / bpm2;
            }
        }
    }

    int maxn = 0, maxi = 0;
    for (std::size_t i = 0; i < m_peaks.size(); ++i) {
        if (bpmn[i] >= maxn) {
            if (bpmn[i] != maxn) {
                maxn = bpmn[i];
                maxi = i;
            }
            if (bpm[maxi] > bpm[i])
                maxi = i;
        }
    }

    if (bpmn[maxi] < 2)
        return 0;
    return bpm[maxi];

    /*********************************************
     number of peaks between similar bpms should be odd
     ideally 1 or 3, 1 is average of other two around it
     3 - center is average, other 2 shold be between outer
     bpm and center(average)
     *********************************************/

    /*
    if(m_peaks.size()) return m_peaks[0].bpm;
    //return 60.0f * m_srate / peakPos;
    return 0;
*/
}
