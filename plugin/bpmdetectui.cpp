// SPDX-License-Identifier: GPL-3.0-or-later
#include <cstdio>

#include "bpmdetectui.h"

START_NAMESPACE_DISTRHO

namespace {
/** Background colour. */
const Color kColorBackground(0.10f, 0.11f, 0.13f);
/** Colour of the tempo readout. */
const Color kColorReadout(0.92f, 0.95f, 1.f);
/** Colour of secondary text. */
const Color kColorSecondary(0.55f, 0.60f, 0.66f);
/** Fill colour of the reset button. */
const Color kColorButton(0.20f, 0.22f, 0.26f);
} // namespace

BpmDetectUi::BpmDetectUi() : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT) {
    const auto scaleFactor = getScaleFactor();
    if (scaleFactor != 1.0) {
        setSize(static_cast<uint>(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor),
                static_cast<uint>(DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor));
    }
    setGeometryConstraints(static_cast<uint>(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor / 2),
                           static_cast<uint>(DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor / 2));
    loadSharedResources();
}

void BpmDetectUi::parameterChanged(uint32_t index, float value) {
    switch (index) {
    case kParameterMinimumBpm:
        minimumBpm_ = value;
        break;
    case kParameterMaximumBpm:
        maximumBpm_ = value;
        break;
    case kParameterDetectedBpm:
        detectedBpm_ = value;
        break;
    default:
        return;
    }
    repaint();
}

Rectangle<float> BpmDetectUi::resetButtonBounds() const {
    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());
    return {width - width * 0.26f, height - height * 0.20f, width * 0.22f, height * 0.14f};
}

void BpmDetectUi::onNanoDisplay() {
    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());

    beginPath();
    rect(0.f, 0.f, width, height);
    fillColor(kColorBackground);
    fill();

    fontFaceId(0);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);

    fontSize(height * 0.12f);
    fillColor(kColorSecondary);
    text(width / 2, height * 0.14f, "Detected BPM", nullptr);

    char readout[16];
    if (detectedBpm_ > 0.f) {
        std::snprintf(readout, sizeof readout, "%.2f", static_cast<double>(detectedBpm_));
    } else {
        std::snprintf(readout, sizeof readout, "--");
    }
    fontSize(height * 0.38f);
    fillColor(kColorReadout);
    text(width / 2, height * 0.48f, readout, nullptr);

    char range[32];
    std::snprintf(range,
                  sizeof range,
                  "Range %.0f-%.0f",
                  static_cast<double>(minimumBpm_),
                  static_cast<double>(maximumBpm_));
    fontSize(height * 0.11f);
    fillColor(kColorSecondary);
    textAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    text(width * 0.05f, height * 0.87f, range, nullptr);

    const auto button = resetButtonBounds();
    beginPath();
    roundedRect(button.getX(), button.getY(), button.getWidth(), button.getHeight(), 3.f);
    fillColor(kColorButton);
    fill();
    fontSize(height * 0.11f);
    fillColor(kColorReadout);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(button.getX() + button.getWidth() / 2,
         button.getY() + button.getHeight() / 2,
         "Reset",
         nullptr);
}

bool BpmDetectUi::onMouse(const MouseEvent &event) {
    if (event.press && event.button == 1 &&
        resetButtonBounds().contains(static_cast<float>(event.pos.getX()),
                                     static_cast<float>(event.pos.getY()))) {
        editParameter(kParameterReset, true);
        setParameterValue(kParameterReset, 1.f);
        editParameter(kParameterReset, false);
        return true;
    }
    return UI::onMouse(event);
}

/** Factory function required by DPF. */
UI *createUI() {
    return new BpmDetectUi();
}

END_NAMESPACE_DISTRHO
