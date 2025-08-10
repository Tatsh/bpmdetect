// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "track.h"

/// WAV audio file 'riff' section header
typedef struct {
    char riff_char[4];
    int package_len;
    char wave[4];
} WavRiff;

/// WAV audio file 'format' section header
typedef struct {
    char fmt[4];
    int format_len;
    short fixed;
    short channel_number;
    int sample_rate;
    int byte_rate;
    short byte_per_sample;
    short bits_per_sample;
} WavFormat;

/// WAV audio file 'data' section header
typedef struct {
    char data_field[4];
    uint data_len;
} WavData;

/// WAV audio file header
typedef struct {
    WavRiff riff;
    WavFormat format;
    WavData data;
} WavHeader;

class TrackWav : public Track {
public:
    TrackWav(const char *filename, bool readtags = true);
    ~TrackWav() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(uint ms) override;
    uint currentPos() override;
    int readSamples(soundtouch::SAMPLETYPE *buffer, unsigned int num) override;

    void storeBPM(std::string sBPM) override;
    void removeBPM() override;

    int readWavHeaders();
    int readHeaderBlock();
    int readRIFFBlock();
    int checkCharTags();
    int read(char *buffer, int maxElems);
    int read(short *buffer, int maxElems);
    int read(float *buffer, int maxElems);

private:
    unsigned long long m_iCurPosBytes;
    FILE *fptr;
    WavHeader header;
};
