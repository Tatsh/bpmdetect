/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
     e-mail             : martin.sakmar@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "audioinput.h"
#include "audioanalyzer.h"

#include <QDebug>

AudioInput::AudioInput(unsigned int samplerate, unsigned int channels) {
    m_pStream = 0;
    setParameters(samplerate, channels);
    PaError paerr = Pa_Initialize();
    if(paerr != paNoError) return;
}


AudioInput::~AudioInput() {
    stop();
    Pa_Terminate();
}

void AudioInput::setParameters(unsigned int samplerate, unsigned int channels) {
    if(m_pStream) return;
    m_uChannels = channels;
    m_uSamplerate = samplerate;
}

void AudioInput::start(AudioAnalyzer* pAnalyzer) {
    stop();
    if(!pAnalyzer) return;
    pAnalyzer->setParameters(m_uSamplerate, m_uChannels);

    PaStreamParameters  inputParameters;
    PaError paerr = paNoError;
    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    inputParameters.channelCount = m_uChannels;          /* stereo input */
    inputParameters.sampleFormat = PA_SAMPLETYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    paerr = Pa_OpenStream(
              &m_pStream,
              &inputParameters,
              NULL,             // &outputParameters
              m_uSamplerate,
              1024,             // frames per buffer
              paClipOff,        // won't output out of range samples so don't bother clipping them
              recordCallback,
              pAnalyzer);
    if(paerr != paNoError) return;

    paerr = Pa_StartStream(m_pStream);
    if(paerr != paNoError) return;
}

void AudioInput::stop() {
    if(!m_pStream) return;

    Pa_AbortStream(m_pStream);
    Pa_CloseStream(m_pStream);
    m_pStream = 0;
}


int AudioInput::recordCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData)
{
    AudioAnalyzer* pAnalyzer = (AudioAnalyzer*) userData;
    const SAMPLE *rptr = (const SAMPLE*) inputBuffer;

    // Prevent unused variable warnings
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if(inputBuffer != NULL) {
        pAnalyzer->process(rptr, framesPerBuffer);
    } else {
        // no buffer
    }

    return paContinue;
}

