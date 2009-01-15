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

#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include "pasample.h"

class AudioAnalyzer;

class AudioInput {
public:
    AudioInput(unsigned int samplerate = 11025, unsigned int channels = 2);
    ~AudioInput();

    void start(AudioAnalyzer* pAnalyzer);
    void stop();

    void setParameters(unsigned int samplerate, unsigned int channels);

protected:
    static int recordCallback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData );

private:
    unsigned int m_uSamplerate,
                 m_uChannels;
    PaStream* m_pStream;
};

#endif
