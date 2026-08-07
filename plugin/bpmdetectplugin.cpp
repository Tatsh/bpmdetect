// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>

#include <BPMDetect.h>

#include "bpmdetectplugin.h"

START_NAMESPACE_DISTRHO

static_assert(std::is_same_v<soundtouch::SAMPLETYPE, float>,
              "SoundTouch must be built with floating-point samples.");

namespace {
/** How long the worker sleeps when the ring buffer is empty. */
constexpr std::chrono::milliseconds kWorkerSleepInterval{20};
} // namespace

BpmDetectPlugin::BpmDetectPlugin()
    : Plugin(kParameterCount, 0, 0), ringBuffer_(kRingBufferCapacity) {
    currentSampleRate_.store(getSampleRate(), std::memory_order_relaxed);
    interleaveBuffer_.resize(std::max<std::size_t>(getBufferSize(), kFallbackBufferFrames) *
                             static_cast<std::size_t>(kDetectionChannels));
    workerThread_ = std::thread(&BpmDetectPlugin::workerLoop, this);
}

BpmDetectPlugin::~BpmDetectPlugin() {
    workerShouldExit_.store(true, std::memory_order_release);
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

const char *BpmDetectPlugin::getLabel() const {
    return "bpmdetect";
}

const char *BpmDetectPlugin::getDescription() const {
    return "Automatic BPM detection analyzer. Reports the estimated tempo of the incoming audio.";
}

const char *BpmDetectPlugin::getMaker() const {
    return "Tatsh";
}

const char *BpmDetectPlugin::getHomePage() const {
    return "https://tatsh.github.io/bpmdetect/";
}

const char *BpmDetectPlugin::getLicense() const {
    return "GPL-3.0-or-later";
}

uint32_t BpmDetectPlugin::getVersion() const {
    return d_version(0, 8, 11);
}

int64_t BpmDetectPlugin::getUniqueId() const {
    return d_cconst('b', 'p', 'm', 'd');
}

void BpmDetectPlugin::initParameter(uint32_t index, Parameter &parameter) {
    switch (index) {
    case kParameterMinimumBpm:
        parameter.hints = kParameterIsAutomatable;
        parameter.name = "Minimum BPM";
        parameter.symbol = "minimum_bpm";
        parameter.unit = "BPM";
        parameter.ranges.def = kDefaultMinimumBpm;
        parameter.ranges.min = kBpmRangeLowerLimit;
        parameter.ranges.max = kBpmRangeUpperLimit;
        break;
    case kParameterMaximumBpm:
        parameter.hints = kParameterIsAutomatable;
        parameter.name = "Maximum BPM";
        parameter.symbol = "maximum_bpm";
        parameter.unit = "BPM";
        parameter.ranges.def = kDefaultMaximumBpm;
        parameter.ranges.min = kBpmRangeLowerLimit;
        parameter.ranges.max = kBpmRangeUpperLimit;
        break;
    case kParameterDetectedBpm:
        parameter.hints = kParameterIsOutput;
        parameter.name = "Detected BPM";
        parameter.symbol = "detected_bpm";
        parameter.unit = "BPM";
        parameter.ranges.def = 0.f;
        parameter.ranges.min = 0.f;
        parameter.ranges.max = kBpmRangeUpperLimit;
        break;
    case kParameterReset:
        parameter.hints = kParameterIsAutomatable | kParameterIsTrigger;
        parameter.name = "Reset";
        parameter.symbol = "reset";
        parameter.ranges.def = 0.f;
        parameter.ranges.min = 0.f;
        parameter.ranges.max = 1.f;
        break;
    }
}

float BpmDetectPlugin::getParameterValue(uint32_t index) const {
    switch (index) {
    case kParameterMinimumBpm:
        return minimumBpm_.load(std::memory_order_relaxed);
    case kParameterMaximumBpm:
        return maximumBpm_.load(std::memory_order_relaxed);
    case kParameterDetectedBpm:
        return detectedBpm_.load(std::memory_order_acquire);
    case kParameterReset:
    default:
        return 0.f;
    }
}

void BpmDetectPlugin::setParameterValue(uint32_t index, float value) {
    switch (index) {
    case kParameterMinimumBpm:
        minimumBpm_.store(std::clamp(value, kBpmRangeLowerLimit, kBpmRangeUpperLimit),
                          std::memory_order_relaxed);
        break;
    case kParameterMaximumBpm:
        maximumBpm_.store(std::clamp(value, kBpmRangeLowerLimit, kBpmRangeUpperLimit),
                          std::memory_order_relaxed);
        break;
    case kParameterReset:
        if (value > 0.5f) {
            resetRequested_.store(true, std::memory_order_release);
        }
        break;
    }
}

void BpmDetectPlugin::activate() {
    resetRequested_.store(true, std::memory_order_release);
}

void BpmDetectPlugin::run(const float **inputs, float **outputs, uint32_t frames) {
    const auto *leftInput = inputs[0];
    const auto *rightInput = inputs[1];
    if (outputs[0] != leftInput) {
        std::memcpy(outputs[0], leftInput, sizeof(float) * frames);
    }
    if (outputs[1] != rightInput) {
        std::memcpy(outputs[1], rightInput, sizeof(float) * frames);
    }

    const auto bufferFrames =
        interleaveBuffer_.size() / static_cast<std::size_t>(kDetectionChannels);
    std::size_t offset = 0;
    while (offset < frames) {
        const auto blockFrames = std::min<std::size_t>(frames - offset, bufferFrames);
        for (std::size_t i = 0; i < blockFrames; ++i) {
            interleaveBuffer_[i * 2] = leftInput[offset + i];
            interleaveBuffer_[i * 2 + 1] = rightInput[offset + i];
        }
        // A full ring buffer drops the block; losing audio is preferable to blocking here.
        ringBuffer_.write(std::span<const float>(
            interleaveBuffer_.data(), blockFrames * static_cast<std::size_t>(kDetectionChannels)));
        offset += blockFrames;
    }
}

void BpmDetectPlugin::bufferSizeChanged(uint32_t newBufferSize) {
    interleaveBuffer_.resize(std::max<std::size_t>(newBufferSize, kFallbackBufferFrames) *
                             static_cast<std::size_t>(kDetectionChannels));
}

void BpmDetectPlugin::sampleRateChanged(double newSampleRate) {
    currentSampleRate_.store(newSampleRate, std::memory_order_relaxed);
    resetRequested_.store(true, std::memory_order_release);
}

float BpmDetectPlugin::foldBpmIntoRange(float bpm) const {
    auto minimum = minimumBpm_.load(std::memory_order_relaxed);
    auto maximum = maximumBpm_.load(std::memory_order_relaxed);
    if (minimum > maximum) {
        std::swap(minimum, maximum);
    }
    if (bpm < 1.f) {
        return 0.f;
    }
    while (bpm > maximum) {
        bpm /= 2.f;
    }
    while (bpm < minimum) {
        bpm *= 2.f;
    }
    return bpm;
}

void BpmDetectPlugin::workerLoop() {
    std::vector<float> chunk(kWorkerChunkSamples);
    std::unique_ptr<soundtouch::BPMDetect> detector;
    auto detectorSampleRate = 0.;
    std::size_t framesSinceEstimate = 0;
    while (!workerShouldExit_.load(std::memory_order_acquire)) {
        const auto sampleRate = currentSampleRate_.load(std::memory_order_relaxed);
        if (resetRequested_.exchange(false, std::memory_order_acq_rel) || !detector ||
            detectorSampleRate != sampleRate) {
            detectorSampleRate = sampleRate;
            detector = std::make_unique<soundtouch::BPMDetect>(
                kDetectionChannels, static_cast<int>(detectorSampleRate));
            ringBuffer_.discardAll();
            detectedBpm_.store(0.f, std::memory_order_release);
            framesSinceEstimate = 0;
        }
        const auto samplesRead = ringBuffer_.read(std::span<float>(chunk));
        if (samplesRead == 0) {
            std::this_thread::sleep_for(kWorkerSleepInterval);
            continue;
        }
        const auto framesRead = samplesRead / static_cast<std::size_t>(kDetectionChannels);
        detector->inputSamples(chunk.data(), static_cast<int>(framesRead));
        framesSinceEstimate += framesRead;
        if (framesSinceEstimate >= static_cast<std::size_t>(detectorSampleRate)) {
            detectedBpm_.store(foldBpmIntoRange(detector->getBpm()), std::memory_order_release);
            framesSinceEstimate = 0;
        }
    }
}

/** Factory function required by DPF. */
Plugin *createPlugin() {
    return new BpmDetectPlugin();
}

END_NAMESPACE_DISTRHO
