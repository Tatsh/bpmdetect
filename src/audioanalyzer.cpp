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
#include "audioanalyzer.h"
#include <beatinfo.h>
#include <bpmcounter.h>
#ifndef NO_GUI
  #include "waveform.h"
#endif

#include <math.h>
#include <cstdlib>
#include <cstring>

#include <QDebug>
#include <QList>

typedef unsigned long ulong;

AudioAnalyzer::AudioAnalyzer(QObject* parent) : QObject(parent) {
    m_fRMSVolR = m_fRMSVolL = 0;
    fftsize = 512;
    fftcfg = 0;
    m_magvector = 0;
    m_instantBufSamples = 0;
    m_pInstantBuffer = m_pPrevInstantBuffer = 0;
    bbeat = false;
    m_pBeat = 0;
    m_pCounter = new BPMCounter();

    for(int i = 0; i < NUMDETECTORS; ++i) {
        m_pBeatDetector[i] = new EnergyBeatDetector(10);
        m_pBeatDetector[i]->setThreshold(5);
    }
#ifndef NO_GUI
    m_pWaveform = new Waveform(100);
    m_pEnergyWave = new Waveform(1);
#endif
    setSamplerate(1000);
    setChannels(1);
    reinit();
}


AudioAnalyzer::~AudioAnalyzer() {
    if(fftcfg) free(fftcfg);
    if(m_magvector) delete [] m_magvector;

    if(m_pInstantBuffer) delete [] m_pInstantBuffer;
    if(m_pPrevInstantBuffer) delete [] m_pPrevInstantBuffer;
    for(int i = 0; i < NUMDETECTORS; ++i) {
        delete m_pBeatDetector[i];
    }
#ifndef NO_GUI
    delete m_pWaveform;
    delete m_pEnergyWave;
#endif

    if(m_pBeat) delete m_pBeat;
    delete m_pCounter;
}

void AudioAnalyzer::setSamplerate(unsigned int samplerate) {
    m_uSamplerate = samplerate;
}

unsigned int AudioAnalyzer::samplerate() const {
    return m_uSamplerate;
}

void AudioAnalyzer::setChannels(unsigned int channels) {
    if(channels < 1) channels = 1;
    if(channels > 2) channels = 2;
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

void AudioAnalyzer::process(const SAMPLE * inputBuffer, ulong size) {
    size -= size % m_uChannels;

    // calculate RMS volumes for VU meters
    calculateRMS(inputBuffer, size);

    for(ulong idx = 0; idx < size / m_uChannels; ++idx) {
        // mix to mono if input is stereo
        SAMPLE sample = inputBuffer[idx*m_uChannels];
        if(m_uChannels == 2) sample = (sample + inputBuffer[idx*m_uChannels+1]) / 2;

        // add sample to instant buffer
        m_pInstantBuffer[m_instantBufSamples++] = sample;

        // analyze instant buffer if it's full
        if(m_instantBufSamples == m_instantBufSize) {
            bool oldbeat = bbeat;
            analyze(m_pInstantBuffer, m_instantBufSize, m_pPrevInstantBuffer);
#ifndef NO_GUI
            bool beat = false;
            if(bbeat == true && oldbeat == false) beat = true;
            m_pWaveform->update(m_pInstantBuffer, m_instantBufSize, beat, -1);
#endif
            // reset the number of samples in instant buffer
            m_instantBufSamples = 0;

            // swap buffers
            SAMPLE* tmp = m_pInstantBuffer;
            m_pInstantBuffer = m_pPrevInstantBuffer;
            m_pPrevInstantBuffer = tmp;
        }
    }

    emit updated();
}

void AudioAnalyzer::calculateRMS(const SAMPLE* inputBuffer, ulong size) {
    float rmssumL = 0, rmssumR = 0;

    for(int i = 0; i < size/m_uChannels; ++i) {
        if(m_uChannels == 1) {
            SAMPLE val = inputBuffer[i];
            if(val < 0) val = -val;
            rmssumL += val;
            rmssumR = rmssumL;
        } else { // 2 channels
            SAMPLE vall, valr;
            vall = inputBuffer[i*2];
            valr = inputBuffer[i*2+1];
            if(vall < 0) vall = -vall;
            if(valr < 0) valr = -valr;
            rmssumL += vall;
            rmssumR += valr;
        }
    }

    m_fRMSVolL = log10(rmssumL/((size/m_uChannels)*1000)+1);
    m_fRMSVolR = log10(rmssumR/((size/m_uChannels)*1000)+1);
}

int AudioAnalyzer::getVuMeterValueL() const {
    return (int) 100*m_fRMSVolL;
}

int AudioAnalyzer::getVuMeterValueR() const {
    return (int) 100*m_fRMSVolR;
}

int AudioAnalyzer::getVuMeterValue() const {
    return (getVuMeterValueL() + getVuMeterValueR()) / 2;
}

const float* AudioAnalyzer::getMagnitude() const {
    return m_magvector;
}

int AudioAnalyzer::getFFTSize() const {
    return fftsize;
}

void AudioAnalyzer::analyze(const SAMPLE* buffer, ulong size, const SAMPLE* prevbuffer) {
    ulong dsize = size;
    if(prevbuffer) dsize = 2*size;
    kiss_fft_scalar inputData[dsize];

    // copy buffer
    for(ulong i = 0; i < size; ++i) {
        if(!prevbuffer) {
            inputData[i] = buffer[i];
            inputData[i] /= SAMPLE_MAXVALUE;
        } else {
            inputData[i] = prevbuffer[i];
            inputData[i+size] = buffer[i+size];
            inputData[i] /= SAMPLE_MAXVALUE;
            inputData[i+size] /= SAMPLE_MAXVALUE;
        }
    }

    kiss_fft_cpx freqdata[fftsize];
    kiss_fftr(fftcfg, inputData, freqdata);

    for(int i = 0; i < fftsize; ++i) {
        m_magvector[i] = sqrt(freqdata[i].r*freqdata[i].r + freqdata[i].i*freqdata[i].i);
    }

    // update beat detectors
    // FIXME: do not use constants and check for NUMDETECTORS
    int start = 1, stop = 3; float energy = 0;
    if(NUMDETECTORS > 0) {
        for(int i = start; i < stop; ++i) energy += m_magvector[i];
        energy /= stop - start;
        m_pBeatDetector[0]->addValue(energy);
    }

    if(NUMDETECTORS > 1) {
        start = 1; stop = 4; energy = 0;
        for(int i = start; i < stop; ++i) energy += m_magvector[i];
        energy /= stop - start;
        m_pBeatDetector[1]->addValue(energy);
    }

    if(NUMDETECTORS > 2) {
        start = 1; stop = 5; energy = 0;
        for(int i = start; i < stop; ++i) energy += m_magvector[i];
        energy /= stop - start;
        m_pBeatDetector[2]->addValue(energy);
    }

    if(NUMDETECTORS > 3) {
        start = 0; stop = 6; energy = 0;
        for(int i = start; i < stop; ++i) energy += m_magvector[i];
        energy /= stop - start;
        m_pBeatDetector[3]->addValue(energy);
    }

    if(NUMDETECTORS > 4) {
        start = 0; stop = 10; energy = 0;
        for(int i = start; i < stop; ++i) energy += m_magvector[i];
        energy /= stop - start;
        m_pBeatDetector[4]->addValue(energy);
    }

    bool tmpbeat = bbeat;
    bbeat = false;
    energy = 0;
    if(!m_pBeat) m_pBeat = new BeatInfo();
    for(int i = 0; i < NUMDETECTORS; ++i) {
        bbeat = bbeat || m_pBeatDetector[i]->isBeat();
        if(m_pBeatDetector[i]->beat() > 0) 
            m_pBeat->addEnergy(m_pBeatDetector[i]->beat());
            //energy += m_pBeatDetector[i]->getCurrentValue();
    }

    energy = 0;
    float maxfreq = 400.0f;
    int maxmagidx = (int) ((maxfreq / ((float)m_uSamplerate / (float) fftsize)));
    for(int i = 0; i <= maxmagidx; ++i) {
        energy += m_magvector[i];
    }
    //energy /= maxmagidx;
    m_pEnergyWave->update(&energy, 1);

    if(tmpbeat != bbeat) {
        emit beat(bbeat);
        if(bbeat) {
            m_pBeat->setStart();
        } else {
            m_pBeat->setEnd();
            //m_pCounter->addBeat(m_pBeat);
        /*
            qDebug() << "Beat start:" << m_pBeat->start()
                     << ", length:" << m_pBeat->length()
                     << ", energy:" << m_pBeat->energy()
                     << "BPM = " << m_pCounter->lastBPM();
        */
            m_pBeat = 0;
        }
    }
}

#ifndef NO_GUI
Waveform* AudioAnalyzer::waveform() const {
    return m_pWaveform;
}

Waveform* AudioAnalyzer::energyWave() const {
    return m_pEnergyWave;
}
#endif

#ifndef NO_GUI
EnergyBeatDetector* AudioAnalyzer::beatDetector(int idx) const {
    if(idx < 0 || idx >= NUMDETECTORS) return 0;
    return m_pBeatDetector[idx];
}
#endif

void AudioAnalyzer::reinit() {
    if(fftcfg) free(fftcfg);
    const float bps = 44100. / 1024.; // buffers per second
    //const float bps = 44100. / 512.; // buffers per second

    m_instantBufSize = m_uSamplerate / bps;
    m_instantBufSize -= m_instantBufSize % 2;
    m_instantBufSamples = 0;
    m_pInstantBuffer = (SAMPLE*) realloc(m_pInstantBuffer, m_instantBufSize * sizeof(SAMPLE));
    m_pPrevInstantBuffer = (SAMPLE*) realloc(m_pPrevInstantBuffer, m_instantBufSize * sizeof(SAMPLE));
    memset(m_pPrevInstantBuffer, 0, m_instantBufSize * sizeof(SAMPLE));

    for(int i = 0; i < NUMDETECTORS; ++i) {
        m_pBeatDetector[i]->setBufferSize((ulong) (bps * 0.3));
    }

    fftsize = m_instantBufSize;
    m_pWaveform->setBufferSize(fftsize);
    m_pWaveform->setSamplerate(samplerate());
    m_pWaveform->setLength(4);

    m_pEnergyWave->setBufferSize(1);
    m_pEnergyWave->setSamplerate(bps);
    m_pEnergyWave->setLength(4);

    fftcfg = kiss_fftr_alloc(fftsize, 0, 0, 0);
    m_magvector = (float*) realloc(m_magvector, fftsize * sizeof(float));
}
