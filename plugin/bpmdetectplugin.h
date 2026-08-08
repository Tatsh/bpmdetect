// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include <DistrhoPlugin.hpp>

#include "parameters.h"
#include "sampleringbuffer.h"

START_NAMESPACE_DISTRHO

/**
 * Analyser plugin that estimates the tempo of incoming audio with SoundTouch's BPM detector.
 *
 * Audio passes through unmodified. The real-time thread only copies samples into a lock-free ring
 * buffer; a worker thread feeds the detector and publishes the estimate through the read-only
 * 'Detected BPM' output parameter, folded into the configured tempo range by halving or doubling.
 */
class BpmDetectPlugin : public Plugin {
public:
    BpmDetectPlugin();
    ~BpmDetectPlugin() override;

protected:
    /** Returns the plugin label. */
    const char *getLabel() const override;
    /** Returns the plugin description. */
    const char *getDescription() const override;
    /** Returns the author name. */
    const char *getMaker() const override;
    /** Returns the project homepage. */
    const char *getHomePage() const override;
    /** Returns the licence identifier. */
    const char *getLicense() const override;
    /** Returns the plugin version. */
    uint32_t getVersion() const override;
    /** Returns the unique identifier used by some plugin formats. */
    int64_t getUniqueId() const override;
    /** Describes a parameter to the host. */
    void initParameter(uint32_t index, Parameter &parameter) override;
    /** Returns the current value of a parameter. */
    float getParameterValue(uint32_t index) const override;
    /** Applies a parameter change from the host. */
    void setParameterValue(uint32_t index, float value) override;
    /** Prepares for processing; requests a fresh detector. */
    void activate() override;
    /** Processes one block: passes audio through and queues it for analysis. */
    void run(const float **inputs, float **outputs, uint32_t frames) override;
    /** Resizes the interleave buffer to match the host buffer size. */
    void bufferSizeChanged(uint32_t newBufferSize) override;
    /** Records the new sample rate and requests a fresh detector. */
    void sampleRateChanged(double newSampleRate) override;

private:
    // Folds a raw estimate into the configured tempo range by halving or doubling.
    float foldBpmIntoRange(float bpm) const;
    // Body of workerThread_: drains the ring buffer and updates the estimate.
    void workerLoop();

    std::atomic<float> detectedBpm_{0.f};
    std::atomic<float> minimumBpm_{kDefaultMinimumBpm};
    std::atomic<float> maximumBpm_{kDefaultMaximumBpm};
    std::atomic<double> currentSampleRate_{0.};
    std::atomic<bool> resetRequested_{true};
    std::atomic<bool> workerShouldExit_{false};
    SampleRingBuffer ringBuffer_;
    std::vector<float> interleaveBuffer_;
    std::thread workerThread_;
};

END_NAMESPACE_DISTRHO
