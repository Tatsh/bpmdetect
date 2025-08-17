// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include <STTypes.h>

#include "utils.h"

class AbstractBpmDetector : public QObject {
    Q_OBJECT
public:
    explicit AbstractBpmDetector(QObject *parent = nullptr);
    ~AbstractBpmDetector();
    /** Add samples to the BPM detector. */
    virtual void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) const = 0;
    /** Get the BPM value. */
    virtual bpmtype getBpm() const = 0;
};
