// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <list>

#include <QElapsedTimer>
#include <QTime>

#define BUFFER_SIZE 15

class BeatInfo;

class BPMCounter {
public:
    BPMCounter();
    ~BPMCounter();
    static double correctBPM(
        double dBPM, float min = 50., float max = 200., bool blimit = false, double rBPM = 0);

    void reset();
    void addBeat();
    float getBPM() const;
    float getError() const;
    long getBeatCount() const;

    void setMinBPM(unsigned int minBPM = 45);
    void setMaxBPM(unsigned int maxBPM = 250);

protected:
    void calcBPM();

private:
    long m_beatCount;

    float m_fBPM, m_fError;
    QElapsedTimer m_qtime;
    QTime m_qstarttime;

    unsigned int m_minBPM, m_maxBPM;
    float m_bpmbuffer[BUFFER_SIZE];
};
