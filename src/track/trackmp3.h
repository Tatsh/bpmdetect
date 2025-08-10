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
    TrackMp3(const char *filename, bool readtags = true);
    ~TrackMp3() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(uint ms) override;
    uint currentPos() override;
    int readSamples(soundtouch::SAMPLETYPE *buffer, unsigned int num) override;
    inline signed int madScale(mad_fixed_t sample);

    void storeBPM(std::string sBPM) override;
    void removeBPM() override;

    void clearFrameList();
    inline unsigned long madLength();

private:
    unsigned long discard(unsigned long samples_wanted);
    int findFrame(int pos);

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
     * To have precise seek within a limited range from the current decode position, we keep track
     * of past decodeded frame, and their exact position. If a seek occours and it is within the
     * range of frames we keep track of a precise seek occours, otherwise an unprecise seek is performed
     */
    std::vector<MadSeekFrameType *> m_qSeekList;
    FILE *fptr;
    unsigned char *inputbuf;
    unsigned int inputbuf_len;
    int framecount;
    int currentframe;
    int bitrate;
    int rest;
    int m_iAvgFrameSize;
    unsigned long long m_iCurPosPCM;
    mad_timer_t pos;
    mad_timer_t filelength;
    mad_stream stream;
    mad_frame *frame;
    mad_synth synth;
};
