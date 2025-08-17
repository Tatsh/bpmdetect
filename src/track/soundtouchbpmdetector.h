// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include "abstractbpmdetector.h"

namespace soundtouch {
    class BPMDetect;
}

class SoundTouchBpmDetector : public AbstractBpmDetector {
    Q_OBJECT
public:
    SoundTouchBpmDetector(int channels, int sampleRate, QObject *parent = nullptr);
    ~SoundTouchBpmDetector() override;
    void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) const override;
    bpmtype getBpm() const override;

private:
    soundtouch::BPMDetect *m_detector;
};
