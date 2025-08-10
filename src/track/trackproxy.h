// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "track.h"

class TrackProxy : public Track {
public:
    TrackProxy(const char *filename, bool readtags = true);
    ~TrackProxy() override;

    void setFilename(const char *filename, bool readtags = true) override;
    void readTags() override;
    void readInfo() override;
    double detectBPM() override;
    void printBPM() override;
    double progress();
    void setBPM(double dBPM) override;
    double getBPM() const override;
    void clearBPM() override;
    void saveBPM() override;
    std::string strBPM() override;
    std::string strBPM(std::string format) override;
    std::string filename() const override;

    /// track length in miliseconds
    unsigned int length() const override;
    std::string strLength() override;
    bool isValid() const override;
    bool isOpened() const override;
    std::string artist() const override;
    std::string title() const override;
    void setRedetect(bool redetect) override;
    bool redetect() const override;
    double progress() const override;
    void setFormat(std::string format = "0.00") override;
    std::string format() const override;
    void enableConsoleProgress(bool enable = true) override;

    void stop() override;

    void setStartPos(uint ms) override;
    uint startPos() const override;
    void setEndPos(uint ms) override;
    uint endPos() const override;
    int samplerate() const override;
    int sampleBytes() const override;
    int sampleBits() const override;
    int channels() const override;
    int trackType() const override;

protected:
    Track *createTrack(const char *filename, bool readtags = true);
    void open() override;
    void close() override;
    void seek(uint ms) override;
    uint currentPos() override;
    int readSamples(std::span<soundtouch::SAMPLETYPE> sp) override;
    void storeBPM(std::string sBPM) override;
    void removeBPM() override;

private:
    Track *m_pTrack;
    bool m_bConsoleProgress;
};
