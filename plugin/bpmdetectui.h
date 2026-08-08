// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include <cstdint>

#include <DistrhoUI.hpp>

#include "parameters.h"

START_NAMESPACE_DISTRHO

/**
 * Minimal UI for the tempo analyser plugin.
 *
 * Shows the current estimate as a large readout together with the configured tempo range, and
 * offers a button that fires the 'Reset' trigger. Exists mainly for hosts whose generic plugin
 * UIs do not display output parameters.
 */
class BpmDetectUi : public UI {
public:
    BpmDetectUi();

protected:
    /** Receives parameter values from the host and repaints. */
    void parameterChanged(uint32_t index, float value) override;
    /** Draws the readout, the tempo range, and the reset button. */
    void onNanoDisplay() override;
    /** Fires the 'Reset' trigger when the reset button is clicked. */
    bool onMouse(const MouseEvent &event) override;

private:
    // Bounds of the reset button for the current window size.
    Rectangle<float> resetButtonBounds() const;

    float detectedBpm_ = 0.f;
    float minimumBpm_ = kDefaultMinimumBpm;
    float maximumBpm_ = kDefaultMaximumBpm;
};

END_NAMESPACE_DISTRHO
