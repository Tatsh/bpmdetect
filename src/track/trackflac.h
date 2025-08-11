// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FLAC/stream_decoder.h"
#include "track.h"

typedef struct {
    FLAC__uint64 total_samples;
    short *buffer;      // buffer of samples (16 bit)
    size_t bufsize;     // buffer size (maximum number of samples)
    uint numsamples;    // number of samples in buffer
    int channels;       // number of channels
    unsigned int srate; // sample rate
    unsigned int bps;   // bits per sample
} FLAC_CLIENT_DATA;

class TrackFlac : public Track {
public:
    TrackFlac(const QString &filename, bool readtags = true);
    ~TrackFlac() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(std::span<soundtouch::SAMPLETYPE> buffer) override;

    void storeBPM(const QString &sBPM) override;
    void removeBPM() override;

private:
    static FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *decoder,
                                                        const FLAC__Frame *frame,
                                                        const FLAC__int32 *const buffer[],
                                                        void *client_data);
    static void metadataCallback(const FLAC__StreamDecoder *decoder,
                                 const FLAC__StreamMetadata *metadata,
                                 void *client_data);
    static void errorCallback(const FLAC__StreamDecoder *decoder,
                              FLAC__StreamDecoderErrorStatus status,
                              void *client_data);

    FLAC__StreamDecoder *m_decoder;
    FLAC_CLIENT_DATA m_cldata = {.total_samples = 0,
                                 .buffer = nullptr,
                                 .bufsize = 0,
                                 .numsamples = 0,
                                 .channels = 0,
                                 .srate = 0,
                                 .bps = 0};

    unsigned long m_ibufidx;
    qint64 m_iCurPosPCM;
};
