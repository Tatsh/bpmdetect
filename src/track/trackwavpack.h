#pragma once

#include <wavpack/wavpack.h>

#include "track.h"

class TrackWavpack : public Track {
public:
    TrackWavpack(const char *filename, bool readtags = true);
    ~TrackWavpack();
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
    WavpackContext *wpc = nullptr;
    unsigned long long m_iCurPosPCM;
};
