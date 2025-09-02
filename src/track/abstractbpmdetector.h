// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include <STTypes.h>

#include "utils.h"

class AbstractBpmDetector : public QObject {
    Q_OBJECT
public:
    /** Constructs a BPM detector. */
    explicit AbstractBpmDetector(QObject *parent = nullptr);
    ~AbstractBpmDetector() override;
    /** Add samples to the BPM detector. */
    virtual void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) = 0;
    /** Get the BPM value. */
    virtual bpmtype getBpm() const = 0;
    /** Reset the class. */
    virtual void reset() = 0;
};
