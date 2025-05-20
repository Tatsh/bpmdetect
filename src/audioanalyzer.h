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

#pragma once

#include "kiss_fftr.h"
#include "pasample.h"

#include <QObject>

#define NUMDETECTORS 5

class EnergyBeatDetector;
class Waveform;
class BeatInfo;
class BPMCounter;
class BPMCalculator;

class AudioAnalyzer : public QObject {
    Q_OBJECT
public:
    AudioAnalyzer(QObject *parent = 0);
    ~AudioAnalyzer();

    unsigned int samplerate() const;
    unsigned int channels() const;
    void setParameters(unsigned int samplerate, unsigned int channels);

    virtual void process(const SAMPLE *inputBuffer, unsigned long size);

    int getVuMeterValueL() const;
    int getVuMeterValueR() const;
    int getVuMeterValue() const;
    const float *getMagnitude() const;
    int getFFTSize() const;

    float getCurrentBPM() const;
    const BPMCalculator *getBPMCalculator() const;

#ifndef NO_GUI
    Waveform *waveform() const;
    EnergyBeatDetector *beatDetector(int idx = 0) const;
    Waveform *calculatorWave() const;
#endif

    Q_SIGNAL void updated();
    Q_SIGNAL void beat(bool);

protected:
    void setSamplerate(unsigned int samplerate);
    void setChannels(unsigned int channels);
    void calculateRMS(const SAMPLE *inputBuffer, unsigned long size);
    void updateWaveform(const SAMPLE *buffer, unsigned long size);
    /// analyze a mono buffer @a buffer of @a size samples
    virtual void analyze(const SAMPLE *buffer, unsigned long size, const SAMPLE *prevbuffer = 0);

private:
    unsigned int m_uSamplerate, m_uChannels;
    float m_fRMSVolL, m_fRMSVolR;
    bool bbeat;
    EnergyBeatDetector *m_pBeatDetector[NUMDETECTORS];

    unsigned long fftsize;
    kiss_fftr_cfg fftcfg;
    float *m_magvector; // current magnitude vector (size = fftsize)

    unsigned long m_instantBufSize;
    unsigned long m_instantBufSamples; ///< number of samples in instant buffer
    SAMPLE *m_pInstantBuffer;          ///< instant buffer (mono)
    SAMPLE *m_pPrevInstantBuffer;

    BeatInfo *m_pBeat; ///< current beat

    unsigned long oldstart;
    BPMCounter *m_pCounter;       ///< BPM counter
    BPMCalculator *m_pCalculator; ///< BPM calculator (autocorrelation)

#ifndef NO_GUI
    Waveform *m_pWaveform;
#endif

    void reinit();
};
