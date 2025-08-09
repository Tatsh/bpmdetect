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

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "bpmcounter.h"
#include "waveform.h"

#define MAX_VALBUFFER_SIZE 4096

Waveform::Waveform(float srate, float length) {
    m_waveformBufSize = m_valueBufSize = 0;
    m_pWaveformBuffer = 0;
    m_pBeatBuffer = 0;
    m_cidx = 0;
    m_maxVal = m_avgSum = 0;
    m_length = 1;
    m_bAverage = false;
    setSamplerate(srate);
    setLength(length);
    setBufferSize();
}

Waveform::~Waveform() {
    if (m_pWaveformBuffer)
        free(m_pWaveformBuffer);
    if (m_pBeatBuffer)
        free(m_pBeatBuffer);
}

void Waveform::setLength(float secs) {
    secs = fabs(secs);
    m_length = secs;
    reinit();
}

void Waveform::setSamplerate(float srate) {
    m_srate = fabs(srate);
    reinit();
}

void Waveform::reinit() {
    // calculate size of waveform buffer from length
    unsigned long size = m_srate * m_length / m_valueBufSize;

    if (size < 10)
        size = 10;
    unsigned long oldsize = m_waveformBufSize;
    m_waveformBufSize = size;
    if (oldsize != size) {
        m_pWaveformBuffer = (float *)realloc(m_pWaveformBuffer, m_waveformBufSize * sizeof(float));
        m_pBeatBuffer = (bool *)realloc(m_pBeatBuffer, m_waveformBufSize * sizeof(bool));
        if (m_pWaveformBuffer)
            memset(m_pWaveformBuffer, 0, m_waveformBufSize * sizeof(float));
        if (m_pBeatBuffer)
            memset(m_pBeatBuffer, 0, m_waveformBufSize * sizeof(bool));
    }
}

void Waveform::setBufferSize(unsigned long bufsize) {
    if (bufsize < 1)
        bufsize = 1;
    if (bufsize > MAX_VALBUFFER_SIZE)
        bufsize = MAX_VALBUFFER_SIZE;
    m_valueBufSize = bufsize;
    reinit();
}

void Waveform::setAverage(bool avg) {
    m_bAverage = avg;
}

void Waveform::update(const SAMPLE *buffer, unsigned long size, bool beat, int beatOffset) {
    if (beatOffset > 0) {
        beat = false;
        beatOffset = 0;
    }
    if (beatOffset + m_waveformBufSize <= 0) {
        beat = false;
        beatOffset = 0;
    }

    for (unsigned long i = 0; i < size; ++i) {
        float val = 1.3 * fabs(buffer[i]) / (SAMPLE_MAXVALUE / 2.0);
        if (val > m_maxVal)
            m_maxVal = val;
        m_avgSum += val;
        if (++m_cidx >= m_valueBufSize) {
            if (m_bAverage)
                addValue(m_avgSum / m_valueBufSize, beat, beatOffset);
            else
                addValue(m_maxVal, beat, beatOffset);
            beat = false;
            m_cidx = 0;
            m_maxVal = 0;
            m_avgSum = 0;
        }
    }
}

void Waveform::update(const float *buffer, unsigned long size) {
    for (unsigned long i = 0; i < size; ++i) {
        float val = fabs(buffer[i]) / 2.0;
        if (val > m_maxVal)
            m_maxVal = val;
        if (++m_cidx >= m_valueBufSize) {
            addValue(m_maxVal, false, 0);
            m_cidx = 0;
            m_maxVal = 0;
        }
    }
}

float Waveform::getMaxValue() {
    float max = 0.1;
    for (unsigned long i = 0; i < size(); ++i) {
        if (m_pWaveformBuffer[i] > max)
            max = m_pWaveformBuffer[i];
    }

    return max;
}

float Waveform::getMinValue() {
    float min = m_pWaveformBuffer[0];
    for (unsigned long i = 0; i < size(); ++i) {
        if (m_pWaveformBuffer[i] < min)
            min = m_pWaveformBuffer[i];
    }

    return min;
}

float Waveform::getAverageValue() {
    float avg = 0;
    for (unsigned long i = 0; i < size(); ++i) {
        avg += m_pWaveformBuffer[i];
    }

    return avg / (float)size();
}

void Waveform::addValue(float val, bool beat, int beatOffset) {
    for (unsigned long i = 0; i < m_waveformBufSize - 1; ++i) {
        m_pWaveformBuffer[i] = m_pWaveformBuffer[i + 1];
        m_pBeatBuffer[i] = m_pBeatBuffer[i + 1];
    }
    m_pWaveformBuffer[m_waveformBufSize - 1] = val;
    m_pBeatBuffer[m_waveformBufSize - 1 + beatOffset] = beat;
}

const float *Waveform::valueBuffer() const {
    return m_pWaveformBuffer;
}

const bool *Waveform::beats() const {
    return m_pBeatBuffer;
}

unsigned long Waveform::size() const {
    return m_waveformBufSize;
}
