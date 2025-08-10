// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "track.h"

class TrackOggVorbis : public Track {
public:
    TrackOggVorbis(const char *filename, bool readtags = true);
    ~TrackOggVorbis() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(uint ms) override;
    uint currentPos() override;
    int readSamples(std::span<soundtouch::SAMPLETYPE> buffer) override;

    void storeBPM(std::string sBPM) override;
    void removeBPM() override;

private:
    unsigned long long m_iCurPosPCM;
    FILE *fptr;
    OggVorbis_File vf;
    int current_section;
};
