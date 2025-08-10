// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <kissfft/kiss_fftr.h>

#include "pasample.h"

#define NUMDETECTORS 5

class EnergyBeatDetector;
class Waveform;
class BeatInfo;
class BPMCounter;
class BPMCalculator;

class AudioAnalyzer : public QObject {
    Q_OBJECT
public:
    AudioAnalyzer(QObject *parent = nullptr);
    ~AudioAnalyzer() override;

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
    virtual void
    analyze(const SAMPLE *buffer, unsigned long size, const SAMPLE *prevbuffer = nullptr);

private:
    unsigned long fftsize;
    unsigned long m_instantBufSize;
    unsigned long m_instantBufSamples; ///< number of samples in instant buffer
    unsigned long oldstart;
    unsigned int m_uSamplerate, m_uChannels;

    EnergyBeatDetector *m_pBeatDetector[NUMDETECTORS];
    kiss_fftr_cfg fftcfg;
    float *m_magvector;       // current magnitude vector (size = fftsize)
    SAMPLE *m_pInstantBuffer; ///< instant buffer (mono)
    SAMPLE *m_pPrevInstantBuffer;
    BeatInfo *m_pBeat;            ///< current beat
    BPMCounter *m_pCounter;       ///< BPM counter
    BPMCalculator *m_pCalculator; ///< BPM calculator (autocorrelation)

#ifndef NO_GUI
    Waveform *m_pWaveform;
#endif

    float m_fRMSVolL, m_fRMSVolR;
    bool bbeat;

    void reinit();
};
