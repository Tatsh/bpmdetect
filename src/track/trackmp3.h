// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <vector>

#include <mad.h>

#include "track.h"

/** Struct used to store mad frames for seeking */
typedef struct MadSeekFrameType {
    unsigned char *m_pStreamPos;
    long int pos;
} MadSeekFrameType;

class TrackMp3 : public Track {
public:
    TrackMp3(const QString &filename, bool readtags = true);
    ~TrackMp3() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;
    inline signed int madScale(mad_fixed_t sample);

    void storeBPM(const QString &sBPM) override;
    void removeBPM() override;

    void clearFrameList();
    inline long madLength();

private:
    unsigned long discard(unsigned long samples_wanted);
    int findFrame(int pos);

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
     * To have precise seek within a limited range from the current decode position, we keep track
     * of past decodeded frame, and their exact position. If a seek occours and it is within the
     * range of frames we keep track of a precise seek occours, otherwise an unprecise seek is performed
     */
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
