// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class BeatInfo {
public:
    BeatInfo();
    ~BeatInfo();

    unsigned long start() const;
    void setStart();
    void setStart(unsigned long msec);
    void setEnd();
    void setEnd(unsigned long msec);
    void setLength(unsigned long len = 1);
    unsigned long length() const;

    float energy() const;
    void setEnergy(float energy);
    void addEnergy(float energy);

    float getBPM(const BeatInfo *bi);

private:
    unsigned long m_start;  /// miliseconds
    unsigned long m_length; /// beat length in miliseconds
    float m_energy;         /// beat energy
};
