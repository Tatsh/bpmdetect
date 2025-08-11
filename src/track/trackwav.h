// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QFile>

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
    TrackWav(const QString &filename, bool readtags = true);
    ~TrackWav() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(std::span<soundtouch::SAMPLETYPE> buffer) override;

    void storeBPM(const QString &sBPM) override;
    void removeBPM() override;

    int readWavHeaders();
    int readHeaderBlock();
    int readRIFFBlock();
    int checkCharTags();
    qint64 read(std::span<char> buffer);
    qint64 read(std::span<short> buffer);
    qint64 read(std::span<float> buffer);

private:
    qint64 m_iCurPosBytes;
    QFile fptr;
    WavHeader header;
};
