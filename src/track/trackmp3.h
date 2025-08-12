// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <vector>

#include <mad.h>

#include "track.h"

/** Struct used to store mad frames for seeking. */
typedef struct MadSeekFrameType {
    unsigned char *m_pStreamPos; //!< Stream position.
    long int pos;                //!< Position in PCM samples.
} MadSeekFrameType;

/** MP3 file. */
class TrackMp3 : public Track {
public:
    /** Constructor.
     * @param filename Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackMp3(const QString &filename, bool readMetadata = true);
    ~TrackMp3() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;
    /**
     * Convert mad fixed point sample to signed int.
     * @param sample Mad fixed point sample.
     * @return Signed int sample.
     */
    inline signed int madScale(mad_fixed_t sample);

    void storeBPM(const QString &sBPM) override;
    void removeBPM() override;

    /** Clear the frame list. */
    void clearFrameList();
    /**
     * Get the length of the MP3 file in samples.
     * @return Length in samples.
     */
    inline long madLength();

private:
    unsigned long discard(unsigned long samples_wanted);
    int findFrame(int pos);

    // It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
    // To have precise seek within a limited range from the current decode position, we keep track
    // of past decodeded frame, and their exact position. If a seek occours and it is within the
    // range of frames we keep track of a precise seek occours, otherwise an unprecise seek is
    // performed.
    QList<MadSeekFrameType *> m_qSeekList;
    QByteArray *inputbuf;
    qsizetype inputbuf_len;
    int framecount;
    int currentframe;
    int bitrate;
    int rest;
    int m_iAvgFrameSize;
    qint64 m_iCurPosPCM;
    mad_timer_t pos;
    mad_timer_t filelength;
    mad_stream stream;
    mad_frame *frame;
    mad_synth synth;
};
