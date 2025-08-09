// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "pasample.h"

class Waveform {
    friend class BPMCalculator;

public:
    /// @a srate is sample rate, @a length is length in seconds
    Waveform(float srate = 44100, float length = 4);
    ~Waveform();

    /// Set source samplerate - used to calculate time
    void setSamplerate(float srate);
    /// Set waveform length in seconds
    void setLength(float secs);
    /// Set size of one buffer used to calculate one value
    void setBufferSize(unsigned long bufsize = 512);
    /// Average values will be used instead of max ones if @a avg is true
    void setAverage(bool avg);
    /// Update should be called always with the same number of samples (size)
    void update(const SAMPLE *buffer, unsigned long size, bool beat = false, int beatOffset = 0);
    void update(const float *buffer, unsigned long size);

    /// @return maximum value of the waveform
    float getMaxValue();
    /// @return minimum waveform value
    float getMinValue();
    /// @return average of all waveform values
    float getAverageValue();

protected:
    /// Return pointer to array of values (use numValues to get size)
    const float *valueBuffer() const;
    const bool *beats() const;
    /// @return waveform size - number of values (length)
    unsigned long size() const;

private:
    float m_srate;  /// sample rate
    float m_length; /// waveform length in seconds

    unsigned long m_valueBufSize; /// size of temporary buffer
    unsigned long m_cidx;         /// samples counter
    float m_maxVal, m_avgSum;
    bool m_bAverage;

    unsigned long m_waveformBufSize; /// size of waveform value buffer
    float *m_pWaveformBuffer;        /// waveform value buffer
    bool *m_pBeatBuffer;             /// beat buffer (same size as waveform buffer)

    /// Initialize buffers
    void reinit();
    /// Shift data to the left and add @a val value to the end
    void addValue(float val, bool beat = false, int beatOffset = 0);
};
