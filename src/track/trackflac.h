// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FLAC/stream_decoder.h"
#include "track.h"

/** Used in the metadata callback. */
typedef struct {
    FLAC__uint64 total_samples; //!< Total number of samples in the file.
    QList<short> *buffer;       //!< Buffer of samples (16 bit).
    qsizetype bufsize;          //!< Buffer size (maximum number of samples).
    uint numsamples;            //!< Number of samples in buffer.
    int channels;               //!< Number of channels.
    unsigned int srate;         //!< Sample rate.
    unsigned int bps;           //!< Bits per sample.
} FLAC_CLIENT_DATA;

/** FLAC file. */
class TrackFlac : public Track {
public:
    /**
     * Constructor.
     * @param filename Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackFlac(const QString &filename, bool readMetadata = true);
    ~TrackFlac() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;

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
    qsizetype m_ibufidx;
    qint64 m_iCurPosPCM;
};
