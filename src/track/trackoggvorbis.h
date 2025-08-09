// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "track.h"

class TrackOggVorbis : public Track {
public:
    TrackOggVorbis(const char *filename, bool readtags = true);
    ~TrackOggVorbis();
    void readTags();

protected:
    void open();
    void close();
    void seek(uint ms);
    uint currentPos();
    int readSamples(soundtouch::SAMPLETYPE *buffer, unsigned int num);

    void storeBPM(std::string sBPM);
    void removeBPM();

private:
    unsigned long long m_iCurPosPCM;
    FILE *fptr;
    OggVorbis_File vf;
    int current_section;
};
