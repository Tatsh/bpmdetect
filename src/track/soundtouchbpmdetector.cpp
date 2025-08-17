// SPDX-License-Identifier: GPL-3.0-or-later
#include "soundtouchbpmdetector.h"

#include <BPMDetect.h>

SoundTouchBpmDetector::SoundTouchBpmDetector(int channels, int sampleRate, QObject *parent)
    : AbstractBpmDetector(parent), m_detector(new soundtouch::BPMDetect(channels, sampleRate)) {
}

SoundTouchBpmDetector::~SoundTouchBpmDetector() {
    delete m_detector;
}

void SoundTouchBpmDetector::inputSamples(const soundtouch::SAMPLETYPE *samples,
                                         int numSamples) const {
    m_detector->inputSamples(samples, numSamples);
}

bpmtype SoundTouchBpmDetector::getBpm() const {
    return m_detector->getBpm();
}
