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
    /** Constructs a SoundTouch-based BPM detector. */
    SoundTouchBpmDetector(QObject *parent = nullptr);
    ~SoundTouchBpmDetector() override;
    void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) override;
    bpmtype getBpm() const override;
    void reset() override;

private:
    soundtouch::BPMDetect *stDetector_ = nullptr;
};
