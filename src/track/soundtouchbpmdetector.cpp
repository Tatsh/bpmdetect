// SPDX-License-Identifier: GPL-3.0-or-later
#include <BPMDetect.h>
#include <QtCore/QDebug>

#include "debug.h"
#include "soundtouchbpmdetector.h"

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
    qCDebug(gLogBpmDetect) << "Resetting SoundTouchBpmDetector with channels:" << channels
                           << ", sample rate:" << sampleRate;
    stDetector_ = new soundtouch::BPMDetect(channels, sampleRate);
}
