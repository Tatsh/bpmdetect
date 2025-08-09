// SPDX-License-Identifier: GPL-3.0-or-later
#include <sys/time.h>

#include <QDebug>

#include "beatinfo.h"

BeatInfo::BeatInfo() {
    m_start = 0;
    m_length = 0;
    m_energy = 0;
}

BeatInfo::~BeatInfo() {
}

void BeatInfo::setStart() {
    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec * 1000 + time.tv_usec / 1000;
    setStart(msec);
}

void BeatInfo::setStart(unsigned long msec) {
    m_start = msec;
    setLength();
}

void BeatInfo::setEnd() {
    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec * 1000 + time.tv_usec / 1000;
    setEnd(msec);
}

void BeatInfo::setEnd(unsigned long msec) {
    if (msec <= m_start)
        setLength();
    else
        setLength(msec - m_start);
}

unsigned long BeatInfo::start() const {
    return m_start;
};
void BeatInfo::setLength(unsigned long len) {
    m_length = len;
};
unsigned long BeatInfo::length() const {
    return m_length;
};
float BeatInfo::energy() const {
    return m_energy;
};
void BeatInfo::setEnergy(float energy) {
    m_energy = energy;
};
void BeatInfo::addEnergy(float energy) {
    m_energy += energy;
};

float BeatInfo::getBPM(const BeatInfo *bi) {
    if (!bi)
        return 0;
    unsigned long diffmsec = 0, s1 = start(), s2 = bi->start();

    if (s1 > s2)
        diffmsec = s1 - s2;
    else if (s2 > s1)
        diffmsec = s2 - s1;
    else
        return 0;

    if (diffmsec > 64e3)
        diffmsec = 86400e3 - diffmsec;
    if (diffmsec > 64e3)
        return 0;

    //qDebug() << "diff =" << diffmsec;
    float bpm = 60000. / diffmsec;
    return bpm;
}
