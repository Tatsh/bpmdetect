// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
// GDI declares a global Rectangle function that would make DGL's Rectangle template ambiguous.
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <shellapi.h>
#endif

#include "bpmdetectui.h"

START_NAMESPACE_DISTRHO

namespace {
/** Background colour. */
const Color kColorBackground(0.10f, 0.11f, 0.13f);
/** Colour of the tempo readout. */
const Color kColorReadout(0.92f, 0.95f, 1.f);
/** Colour of secondary text. */
const Color kColorSecondary(0.55f, 0.60f, 0.66f);
/** Fill colour of the reset button and slider tracks. */
const Color kColorControl(0.20f, 0.22f, 0.26f);
/** Fill colour of the active part of a slider. */
const Color kColorSliderFill(0.35f, 0.55f, 0.85f);
/** Colour of the project page link. */
const Color kColorLink(0.45f, 0.62f, 0.85f);
/** Address of the project page, shown at the bottom left. */
constexpr const char *kProjectUrl = "https://github.com/Tatsh/bpmdetect";

/** Opens the project page in the default browser. */
void openProjectPage() {
#if defined(_WIN32)
    ShellExecuteA(nullptr, "open", kProjectUrl, nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::system("open https://github.com/Tatsh/bpmdetect");
#else
    std::system("xdg-open https://github.com/Tatsh/bpmdetect &");
#endif
}
} // namespace

BpmDetectUi::BpmDetectUi() : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT, true) {
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
    return {width * 0.76f, height * 0.83f, width * 0.19f, height * 0.11f};
}

Rectangle<float> BpmDetectUi::linkBounds() const {
    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());
    return {width * 0.03f, height * 0.83f, width * 0.70f, height * 0.11f};
}

Rectangle<float> BpmDetectUi::sliderTrackBounds(uint32_t index) const {
    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());
    const auto y = index == kParameterMinimumBpm ? height * 0.56f : height * 0.70f;
    return {width * 0.16f, y, width * 0.62f, height * 0.08f};
}

void BpmDetectUi::applySliderPosition(uint32_t index, float x) {
    const auto track = sliderTrackBounds(index);
    const auto position = std::clamp((x - track.getX()) / track.getWidth(), 0.f, 1.f);
    const auto value = kBpmRangeLowerLimit + position * (kBpmRangeUpperLimit - kBpmRangeLowerLimit);
    if (index == kParameterMinimumBpm) {
        minimumBpm_ = value;
    } else {
        maximumBpm_ = value;
    }
    setParameterValue(index, value);
    repaint();
}

void BpmDetectUi::drawSlider(uint32_t index, const char *label, float value) {
    const auto track = sliderTrackBounds(index);
    const auto height = static_cast<float>(getHeight());
    const auto textY = track.getY() + track.getHeight() / 2;

    fontSize(height * 0.09f);
    fillColor(kColorSecondary);
    textAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    text(static_cast<float>(getWidth()) * 0.04f, textY, label, nullptr);

    beginPath();
    roundedRect(track.getX(), track.getY(), track.getWidth(), track.getHeight(), 3.f);
    fillColor(kColorControl);
    fill();

    const auto position =
        (value - kBpmRangeLowerLimit) / (kBpmRangeUpperLimit - kBpmRangeLowerLimit);
    beginPath();
    roundedRect(track.getX(),
                track.getY(),
                track.getWidth() * std::clamp(position, 0.f, 1.f),
                track.getHeight(),
                3.f);
    fillColor(kColorSliderFill);
    fill();

    char valueText[16];
    std::snprintf(valueText, sizeof valueText, "%.0f", static_cast<double>(value));
    fillColor(kColorReadout);
    textAlign(ALIGN_RIGHT | ALIGN_MIDDLE);
    text(static_cast<float>(getWidth()) * 0.96f, textY, valueText, nullptr);
}

void BpmDetectUi::onNanoDisplay() {
    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());

    beginPath();
    rect(0.f, 0.f, width, height);
    fillColor(kColorBackground);
    fill();

    fontFaceId(0);

    fontSize(height * 0.09f);
    fillColor(kColorSecondary);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(width / 2, height * 0.10f, "Detected BPM", nullptr);

    char readout[16];
    if (detectedBpm_ > 0.f) {
        std::snprintf(readout, sizeof readout, "%.2f", static_cast<double>(detectedBpm_));
    } else {
        std::snprintf(readout, sizeof readout, "--");
    }
    fontSize(height * 0.27f);
    fillColor(kColorReadout);
    text(width / 2, height * 0.33f, readout, nullptr);

    drawSlider(kParameterMinimumBpm, "Min", minimumBpm_);
    drawSlider(kParameterMaximumBpm, "Max", maximumBpm_);

    const auto link = linkBounds();
    fontSize(height * 0.06f);
    fillColor(kColorLink);
    textAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    text(link.getX() + width * 0.01f, link.getY() + link.getHeight() / 2, kProjectUrl, nullptr);

    const auto button = resetButtonBounds();
    beginPath();
    roundedRect(button.getX(), button.getY(), button.getWidth(), button.getHeight(), 3.f);
    fillColor(kColorControl);
    fill();
    fontSize(height * 0.08f);
    fillColor(kColorReadout);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(button.getX() + button.getWidth() / 2,
         button.getY() + button.getHeight() / 2,
         "Reset",
         nullptr);
}

bool BpmDetectUi::onMouse(const MouseEvent &event) {
    const auto x = static_cast<float>(event.pos.getX());
    const auto y = static_cast<float>(event.pos.getY());
    if (event.press && event.button == 1) {
        if (resetButtonBounds().contains(x, y)) {
            editParameter(kParameterReset, true);
            setParameterValue(kParameterReset, 1.f);
            editParameter(kParameterReset, false);
            return true;
        }
        if (linkBounds().contains(x, y)) {
            openProjectPage();
            return true;
        }
        for (const auto index : {kParameterMinimumBpm, kParameterMaximumBpm}) {
            auto track = sliderTrackBounds(index);
            // Give the track a little more vertical grab area.
            track.setY(track.getY() - track.getHeight() / 2);
            track.setHeight(track.getHeight() * 2);
            if (track.contains(x, y)) {
                draggedParameter_ = index;
                editParameter(index, true);
                applySliderPosition(index, x);
                return true;
            }
        }
    } else if (!event.press && draggedParameter_ != kParameterCount) {
        editParameter(draggedParameter_, false);
        draggedParameter_ = kParameterCount;
        return true;
    }
    return UI::onMouse(event);
}

bool BpmDetectUi::onMotion(const MotionEvent &event) {
    if (draggedParameter_ != kParameterCount) {
        applySliderPosition(draggedParameter_, static_cast<float>(event.pos.getX()));
        return true;
    }
    return UI::onMotion(event);
}

/** Factory function required by DPF. */
UI *createUI() {
    return new BpmDetectUi();
}

END_NAMESPACE_DISTRHO
