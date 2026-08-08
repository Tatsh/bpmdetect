// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include <cstdint>

#include <DistrhoUI.hpp>

#include "parameters.h"

START_NAMESPACE_DISTRHO

/**
 * UI for the tempo analyser plugin.
 *
 * Shows the current estimate as a large readout, sliders for the tempo range, and a button that
 * fires the 'Reset' trigger. Exists mainly for hosts whose generic plugin UIs do not display
 * output parameters.
 */
class BpmDetectUi : public UI {
public:
    BpmDetectUi();

protected:
    /** Receives parameter values from the host and repaints. */
    void parameterChanged(uint32_t index, float value) override;
    /** Draws the readout, the tempo range sliders, and the reset button. */
    void onNanoDisplay() override;
    /** Starts slider drags, fires the 'Reset' trigger, and opens the project page. */
    bool onMouse(const MouseEvent &event) override;
    /** Updates the dragged slider. */
    bool onMotion(const MotionEvent &event) override;

private:
    // Bounds of the reset button for the current window size.
    Rectangle<float> resetButtonBounds() const;
    // Bounds of the project page link for the current window size.
    Rectangle<float> linkBounds() const;
    // Bounds of a slider track. The index must be kParameterMinimumBpm or kParameterMaximumBpm.
    Rectangle<float> sliderTrackBounds(uint32_t index) const;
    // Applies a horizontal position within a slider track as the parameter value.
    void applySliderPosition(uint32_t index, float x);
    // Draws one slider row with its label and value.
    void drawSlider(uint32_t index, const char *label, float value);

    float detectedBpm_ = 0.f;
    float minimumBpm_ = kDefaultMinimumBpm;
    float maximumBpm_ = kDefaultMaximumBpm;
    // Parameter being dragged, or kParameterCount when no drag is active.
    uint32_t draggedParameter_ = kParameterCount;
};

END_NAMESPACE_DISTRHO
