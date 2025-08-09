// SPDX-License-Identifier: GPL-3.0-or-later
#include <sys/time.h>

#include <QDebug>

#include "metronome.h"

Metronome::Metronome() {
    setInterval(1000);
    stop();
    setSync();
}

Metronome::~Metronome() {
}

void Metronome::setInterval(unsigned long msec) {
    if (msec < 100)
        msec = 100;
    m_interval = msec;
}

void Metronome::setBPM(float bpm) {
    m_interval = (unsigned long)(60000. / bpm);
}

void Metronome::setSync() {
    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec * 1000 + time.tv_usec / 1000;
    setSync(msec);
}

void Metronome::setSync(unsigned long msec) {
    m_syncTime = msec;
}

unsigned long Metronome::progress() const {
    if (!m_bStarted) {
        return 0;
    }

    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec * 1000 + time.tv_usec / 1000;

    unsigned long syncms = m_syncTime % m_interval;
    unsigned long iprogress = (msec + m_interval - syncms) % m_interval;
    return iprogress;
}

float Metronome::progressPercent() const {
    return 100.0 * ((float)progress() / (float)m_interval);
}

void Metronome::start() {
    m_bStarted = true;
}

void Metronome::stop() {
    m_bStarted = false;
}
