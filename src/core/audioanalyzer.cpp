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

#include "audioanalyzer.h"
#include "beatinfo.h"
#include "bpmcalculator.h"
#include "bpmcounter.h"
#include "energybeatdetector.h"
#ifndef NO_GUI
#include "waveform.h"
#endif

#include <cstdlib>
#include <cstring>
#include <math.h>

#include <QDebug>
#include <QList>

typedef unsigned long ulong;

const int bufferParts = 4;

AudioAnalyzer::AudioAnalyzer(QObject *parent) : QObject(parent) {
    m_fRMSVolR = m_fRMSVolL = 0;
    fftsize = 512;
    fftcfg = nullptr;
    m_magvector = nullptr;
    m_instantBufSamples = 0;
    m_pInstantBuffer = m_pPrevInstantBuffer = nullptr;
    bbeat = false;
    m_pBeat = nullptr;
    m_pCounter = new BPMCounter();
    m_pCalculator = new BPMCalculator(5);

    for (int i = 0; i < NUMDETECTORS; ++i) {
        m_pBeatDetector[i] = new EnergyBeatDetector(10);
        m_pBeatDetector[i]->setThreshold(5);
        if (i > 4)
            m_pBeatDetector[i]->setThreshold(1);
    }
#ifndef NO_GUI
    m_pWaveform = new Waveform(100);
#endif
    setSamplerate(1000);
    setChannels(1);
    reinit();
}

AudioAnalyzer::~AudioAnalyzer() {
    if (fftcfg)
        free(fftcfg);
    if (m_magvector)
        free(m_magvector);
    if (m_pInstantBuffer)
        free(m_pInstantBuffer);
    if (m_pPrevInstantBuffer)
        free(m_pPrevInstantBuffer);
    for (int i = 0; i < NUMDETECTORS; ++i) {
        delete m_pBeatDetector[i];
    }
#ifndef NO_GUI
    delete m_pWaveform;
#endif

    if (m_pBeat)
        delete m_pBeat;
    delete m_pCounter;
    delete m_pCalculator;
}

void AudioAnalyzer::setSamplerate(unsigned int samplerate) {
    m_uSamplerate = samplerate;
}

unsigned int AudioAnalyzer::samplerate() const {
    return m_uSamplerate;
}

void AudioAnalyzer::setChannels(unsigned int channels) {
    if (channels < 1)
        channels = 1;
    if (channels > 2)
        channels = 2;
    m_uChannels = channels;
}

unsigned int AudioAnalyzer::channels() const {
    return m_uChannels;
}

void AudioAnalyzer::setParameters(unsigned int samplerate, unsigned int channels) {
    setSamplerate(samplerate);
    setChannels(channels);
    reinit();
}

void AudioAnalyzer::process(const SAMPLE *inputBuffer, ulong size) {
    size -= size % m_uChannels;

    // calculate RMS volumes for VU meters
    calculateRMS(inputBuffer, size);

    for (ulong idx = 0; idx < size / m_uChannels; ++idx) {
        // mix to mono if input is stereo
        SAMPLE sample = inputBuffer[idx * m_uChannels];
        if (m_uChannels == 2)
            sample = (sample + inputBuffer[idx * m_uChannels + 1]) / 2;

        // add sample to instant buffer
        m_pInstantBuffer[m_instantBufSamples++] = sample;

        // analyze instant buffer if it's full
        if (m_instantBufSamples == m_instantBufSize) {
            bool oldbeat = bbeat;
            //analyze(m_pInstantBuffer, m_instantBufSize, m_pPrevInstantBuffer);
            analyze(m_pInstantBuffer, m_instantBufSize, 0);
#ifndef NO_GUI
            bool beat = false;
            if (bbeat == true && oldbeat == false)
                beat = true;
            m_pWaveform->update(m_pInstantBuffer, m_instantBufSize, beat, 0);
#endif
            // update bpmcalculator
            int n = bufferParts;
            float avg[n];
            for (int i = 0; i < n; ++i)
                avg[i] = 0;
            for (ulong i = 0; i < m_instantBufSamples; ++i) {
                if (i < m_instantBufSize) {
                    float val = fabsf(static_cast<float>(m_pInstantBuffer[i]));
                    unsigned long cn = static_cast<unsigned long>(i) /
                                       static_cast<unsigned long>(m_instantBufSamples /
                                                                  static_cast<unsigned long>(n));
                    avg[cn] += val;
                    avg[cn] += val;
                }
            }
            for (int i = 0; i < n; ++i) {
                avg[i] /= static_cast<float>(m_instantBufSamples / n);
                //avg[i] /= static_cast<float>(SAMPLE_MAXVALUE);
            }
            m_pCalculator->update(avg, n);

            // reset the number of samples in instant buffer
            m_instantBufSamples = 0;
            // swap buffers
            SAMPLE *tmp = m_pInstantBuffer;
            m_pInstantBuffer = m_pPrevInstantBuffer;
            m_pPrevInstantBuffer = tmp;
        }
    }

    emit updated();
}

void AudioAnalyzer::calculateRMS(const SAMPLE *inputBuffer, ulong size) {
    float rmssumL = 0, rmssumR = 0;

    for (ulong i = 0; i < size / m_uChannels; ++i) {
        if (m_uChannels == 1) {
            SAMPLE val = inputBuffer[i];
            if (val < 0)
                val = -val;
            rmssumL += val;
            rmssumR = rmssumL;
        } else { // 2 channels
            SAMPLE vall, valr;
            vall = inputBuffer[i * 2];
            valr = inputBuffer[i * 2 + 1];
            if (vall < 0)
                vall = -vall;
            if (valr < 0)
                valr = -valr;
            rmssumL += vall;
            rmssumR += valr;
        }
    }

    m_fRMSVolL = log10(rmssumL / ((size / m_uChannels) * 1000) + 1);
    m_fRMSVolR = log10(rmssumR / ((size / m_uChannels) * 1000) + 1);
}

int AudioAnalyzer::getVuMeterValueL() const {
    return (int)100 * m_fRMSVolL;
}

int AudioAnalyzer::getVuMeterValueR() const {
    return (int)100 * m_fRMSVolR;
}

int AudioAnalyzer::getVuMeterValue() const {
    return (getVuMeterValueL() + getVuMeterValueR()) / 2;
}

const float *AudioAnalyzer::getMagnitude() const {
    return m_magvector;
}

int AudioAnalyzer::getFFTSize() const {
    return fftsize;
}

float AudioAnalyzer::getCurrentBPM() const {
    return m_pCalculator->getBpm();
}

const BPMCalculator *AudioAnalyzer::getBPMCalculator() const {
    return m_pCalculator;
}

void AudioAnalyzer::analyze(const SAMPLE *buffer, ulong size, const SAMPLE *prevbuffer) {
    ulong dsize = size;
    if (prevbuffer)
        dsize = 2 * size;
    kiss_fft_scalar inputData[dsize];

    // copy buffer
    for (ulong i = 0; i < size; ++i) {
        if (!prevbuffer) {
            inputData[i] = buffer[i];
            inputData[i] /= SAMPLE_MAXVALUE;
        } else {
            inputData[i] = prevbuffer[i];
            inputData[i + size] = buffer[i + size];
            inputData[i] /= SAMPLE_MAXVALUE;
            inputData[i + size] /= SAMPLE_MAXVALUE;
        }
    }

    kiss_fft_cpx freqdata[fftsize];
    kiss_fftr(fftcfg, inputData, freqdata);

    for (unsigned long i = 0; i < fftsize; ++i) {
        m_magvector[i] = sqrt(freqdata[i].r * freqdata[i].r + freqdata[i].i * freqdata[i].i);
    }

#define FREQIDX(freq)                                                                              \
    static_cast<int>((freq) / (static_cast<float>(m_uSamplerate) / static_cast<float>(fftsize)))

    // update beat detectors
    float energy = 0;
    int start = FREQIDX(50), stop = FREQIDX(150);
    if (NUMDETECTORS > 0) {
        for (int i = start; i <= stop; ++i)
            energy += m_magvector[i];
        if (stop - start > 1)
            energy /= stop - start;
        m_pBeatDetector[0]->addValue(energy);
    }

    if (NUMDETECTORS > 1) {
        start = FREQIDX(80);
        stop = FREQIDX(180);
        energy = 0;
        for (int i = start; i <= stop; ++i)
            energy += m_magvector[i];
        if (stop - start > 1)
            energy /= stop - start;
        m_pBeatDetector[1]->addValue(energy);
    }

    if (NUMDETECTORS > 2) {
        start = FREQIDX(120);
        stop = FREQIDX(250);
        energy = 0;
        for (int i = start; i <= stop; ++i)
            energy += m_magvector[i];
        if (stop - start > 1)
            energy /= stop - start;
        m_pBeatDetector[2]->addValue(energy);
    }

    if (NUMDETECTORS > 3) {
        start = FREQIDX(180);
        stop = FREQIDX(300);
        energy = 0;
        for (int i = start; i <= stop; ++i)
            energy += m_magvector[i];
        if (stop - start > 1)
            energy /= stop - start;
        m_pBeatDetector[3]->addValue(energy);
    }

    if (NUMDETECTORS > 4) {
        start = FREQIDX(250);
        stop = FREQIDX(500);
        energy = 0;
        for (int i = start; i <= stop; ++i)
            energy += m_magvector[i];
        if (stop - start > 1)
            energy /= stop - start;
        m_pBeatDetector[4]->addValue(energy);
    }

    bool tmpbeat = bbeat;
    bbeat = false;
    energy = 0;
    if (!m_pBeat)
        m_pBeat = new BeatInfo();
    /*
    for(int i = 0; i < NUMDETECTORS; ++i) {
        bbeat = bbeat || m_pBeatDetector[i]->isBeat();
        if(m_pBeatDetector[i]->beat() > 0)
            m_pBeat->addEnergy(m_pBeatDetector[i]->beat());
    }
*/
    bbeat = bbeat || m_pCalculator->isBeat();

    if (tmpbeat != bbeat) {
        emit beat(bbeat);
        if (bbeat) {
            m_pBeat->setStart();
        } else {
            m_pBeat->setEnd();
            //m_pCounter->addBeat(m_pBeat);
            //m_pBeat = 0;
        }
    }
}

#ifndef NO_GUI
Waveform *AudioAnalyzer::waveform() const {
    return m_pWaveform;
}

Waveform *AudioAnalyzer::calculatorWave() const {
    return m_pCalculator->waveform();
}
#endif

#ifndef NO_GUI
EnergyBeatDetector *AudioAnalyzer::beatDetector(int idx) const {
    if (idx < 0 || idx >= NUMDETECTORS)
        return 0;
    return m_pBeatDetector[idx];
}
#endif

void AudioAnalyzer::reinit() {
    if (fftcfg)
        free(fftcfg);
    const float bps = 44100. / 1024.; // buffers per second
    //const float bps = 44100. / 512.; // buffers per second

    m_instantBufSize = static_cast<unsigned long>(static_cast<float>(m_uSamplerate) / bps);
    m_instantBufSize -= m_instantBufSize % 2;
    m_instantBufSamples = 0;
    m_pInstantBuffer =
        static_cast<SAMPLE *>(realloc(m_pInstantBuffer, m_instantBufSize * sizeof(SAMPLE)));
    m_pPrevInstantBuffer =
        (SAMPLE *)realloc(m_pPrevInstantBuffer, m_instantBufSize * sizeof(SAMPLE));
    memset(m_pPrevInstantBuffer, 0, m_instantBufSize * sizeof(SAMPLE));

    for (int i = 0; i < NUMDETECTORS; ++i) {
        m_pBeatDetector[i]->setBufferSize((ulong)(bps * 0.2));
    }

    fftsize = 1024;
    if (fftsize > m_instantBufSize)
        fftsize = m_instantBufSize;
    m_pWaveform->setBufferSize(m_instantBufSize);
    m_pWaveform->setSamplerate(static_cast<float>(samplerate()));
    m_pWaveform->setLength(5);

    fftcfg = kiss_fftr_alloc(static_cast<int>(fftsize), 0, nullptr, nullptr);
    m_magvector = static_cast<float *>(realloc(m_magvector, fftsize * sizeof(float)));

    m_pCalculator->setSamplerate(bps * bufferParts);
    m_pCalculator->setLength(5);
}
