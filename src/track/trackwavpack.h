// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <wavpack/wavpack.h>

#include "track.h"

class TrackWavpack : public Track {
public:
    TrackWavpack(const char *filename, bool readtags = true);
    ~TrackWavpack() override;
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
    WavpackContext *wpc = nullptr;
    unsigned long long m_iCurPosPCM;
};
