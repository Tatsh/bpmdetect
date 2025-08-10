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
    m_interval = static_cast<unsigned long>(60000. / static_cast<double>(bpm));
}

void Metronome::setSync() {
    struct timeval time;
    gettimeofday(&time, nullptr);
    unsigned long msec = static_cast<unsigned long>(static_cast<uint64_t>(time.tv_sec) * 1000UL) +
                         static_cast<unsigned long>(static_cast<uint64_t>(time.tv_usec) / 1000UL);
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
    gettimeofday(&time, nullptr);
    unsigned long msec = static_cast<unsigned long>(static_cast<uint64_t>(time.tv_sec) * 1000UL) +
                         static_cast<unsigned long>(static_cast<uint64_t>(time.tv_usec) / 1000UL);

    unsigned long syncms = m_syncTime % m_interval;
    unsigned long iprogress = (msec + m_interval - syncms) % m_interval;
    return iprogress;
}

float Metronome::progressPercent() const {
    return 100.0f * (static_cast<float>(progress()) / static_cast<float>(m_interval));
}

void Metronome::start() {
    m_bStarted = true;
}

void Metronome::stop() {
    m_bStarted = false;
}
