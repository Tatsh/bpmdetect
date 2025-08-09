// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class Metronome {
public:
#ifdef TESTING
    friend class MetronomeTest;
#endif
    Metronome();
    ~Metronome();

    void setInterval(unsigned long msec);
    void setBPM(float bpm);

    void setSync();
    void setSync(unsigned long msec);
    unsigned long progress() const;
    float progressPercent() const;

    void start();
    void stop();

private:
    unsigned long m_interval;
    unsigned long m_syncTime;
    bool m_bStarted;
};
