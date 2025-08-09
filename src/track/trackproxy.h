// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "track.h"

class TrackProxy : public Track {
public:
    TrackProxy(const char *filename, bool readtags = true);
    ~TrackProxy();

    void setFilename(const char *filename, bool readtags = true);
    void readTags();
    void readInfo();
    double detectBPM();
    void printBPM();
    double progress();
    void setBPM(double dBPM);
    double getBPM() const;
    void clearBPM();
    void saveBPM();
    std::string strBPM();
    std::string strBPM(std::string format);
    std::string filename() const;

    /// track length in miliseconds
    unsigned int length() const;
    std::string strLength();
    bool isValid() const;
    bool isOpened() const;
    std::string artist() const;
    std::string title() const;
    void setRedetect(bool redetect);
    bool redetect() const;
    double progress() const;
    void setFormat(std::string format = "0.00");
    std::string format() const;
    void enableConsoleProgress(bool enable = true);

    void stop();

    void setStartPos(uint ms);
    uint startPos() const;
    void setEndPos(uint ms);
    uint endPos() const;
    int samplerate() const;
    int sampleBytes() const;
    int sampleBits() const;
    int channels() const;
    int trackType() const;

protected:
    Track *createTrack(const char *filename, bool readtags = true);
    void open();
    void close();
    void seek(uint ms);
    uint currentPos();
    int readSamples(soundtouch::SAMPLETYPE *buffer, unsigned int num);
    void storeBPM(std::string sBPM);
    void removeBPM();

private:
    Track *m_pTrack;
    bool m_bConsoleProgress;
};
