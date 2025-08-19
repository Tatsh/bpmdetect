// SPDX-License-Identifier: GPL-3.0-or-later
#include "soundtouchbpmdetector.h"

#include <BPMDetect.h>

SoundTouchBpmDetector::SoundTouchBpmDetector(QObject *parent) : AbstractBpmDetector(parent) {
}

SoundTouchBpmDetector::~SoundTouchBpmDetector() {
    delete stDetector_;
}

void SoundTouchBpmDetector::inputSamples(const soundtouch::SAMPLETYPE *samples,
                                         int numSamples) const {
    stDetector_->inputSamples(samples, numSamples);
}

bpmtype SoundTouchBpmDetector::getBpm() const {
    return stDetector_->getBpm();
}

void SoundTouchBpmDetector::reset(int channels, int sampleRate) {
    if (stDetector_) {
        delete stDetector_;
    }
    stDetector_ = new soundtouch::BPMDetect(channels, sampleRate);
}
