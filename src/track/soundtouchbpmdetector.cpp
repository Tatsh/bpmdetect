// SPDX-License-Identifier: GPL-3.0-or-later
#include <BPMDetect.h>
#include <QtCore/QDebug>

#include "constants.h"
#include "debug.h"
#include "soundtouchbpmdetector.h"

SoundTouchBpmDetector::SoundTouchBpmDetector(QObject *parent) : AbstractBpmDetector(parent) {
}

SoundTouchBpmDetector::~SoundTouchBpmDetector() {
    delete stDetector_;
    stDetector_ = nullptr;
}

void SoundTouchBpmDetector::inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) {
    if (!stDetector_) {
        // LCOV_EXCL_START
        qCWarning(gLogBpmDetect) << "Re-initialising SoundTouch BPM detector in inputSamples().";
        stDetector_ = new soundtouch::BPMDetect(DETECTION_CHANNELS, DETECTION_SAMPLE_RATE);
        // LCOV_EXCL_STOP
    }
    stDetector_->inputSamples(samples, numSamples);
}

bpmtype SoundTouchBpmDetector::getBpm() const {
    if (!stDetector_) {
        // LCOV_EXCL_START
        qCCritical(gLogBpmDetect) << "SoundTouchBpmDetector: No BPM detector initialized.";
        return -1;
        // LCOV_EXCL_STOP
    }
    return static_cast<bpmtype>(stDetector_->getBpm());
}

void SoundTouchBpmDetector::reset() {
    if (stDetector_) {
        delete stDetector_;
        stDetector_ = nullptr;
    }
    stDetector_ = new soundtouch::BPMDetect(DETECTION_CHANNELS, DETECTION_SAMPLE_RATE);
}
