// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QtCore/QFile>

#include "track.h"

/** WAV audio file 'riff' section header. */
typedef struct {
    char riff_char[4]; //!< 'RIFF' field.
    int package_len;   //!< File size minus 8 bytes.
    char wave[4];      //!< 'WAVE' field.
} WavRiff;

/** WAV audio file 'format' section header. */
typedef struct {
    char fmt[4];           //!< 'fmt ' field.
    int format_len;        //!< Length of the format field.
    short fixed;           //!< Fixed format (1 = PCM).
    short channel_number;  //!< Number of channels.
    int sample_rate;       //!< Sample rate.
    int byte_rate;         //!< Byte rate.
    short byte_per_sample; //!< Bytes per sample.
    short bits_per_sample; //!< Bits per sample.
} WavFormat;

/** WAV audio file 'data' section header. */
typedef struct {
    char data_field[4];    //!< 'data' field.
    unsigned int data_len; //!< Length of the data.
} WavData;

/** WAV audio file header. */
typedef struct {
    WavRiff riff;     //!< 'riff' section header.
    WavFormat format; //!< Format section.
    WavData data;     //!< Data.
} WavHeader;

/** WAVE file. */
class TrackWav : public Track {
public:
    /** Constructor.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackWav(const QString &fileName, bool readMetadata = true);
    ~TrackWav() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(quint64 ms) override;
    quint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;
    void storeBPM(const QString &sBPM) override;
    void removeBpm() override;

private:
    int readWavHeaders();
    int readHeaderBlock();
    int readRIFFBlock();
    int checkCharTags();
    qint64 read(QSpan<char> buffer);
    qint64 read(QSpan<short> buffer);
    qint64 read(QSpan<float> buffer);

    quint64 m_iCurPosBytes;
    QFile fptr;
    WavHeader header;
};
