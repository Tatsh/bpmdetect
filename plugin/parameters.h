// SPDX-License-Identifier: GPL-3.0-or-later
/** @file Parameter indices and shared constants for the plugin and its UI. */
#pragma once
#include <cstddef>
#include <cstdint>

#include <DistrhoDetails.hpp>

START_NAMESPACE_DISTRHO

/** Parameter indices. */
enum ParameterId : uint32_t {
    kParameterMinimumBpm = 0,
    kParameterMaximumBpm,
    kParameterDetectedBpm,
    kParameterReset,
    kParameterCount
};

/** Number of channels fed to the detector. */
inline constexpr int kDetectionChannels = 2;
/** Default lower bound of the tempo range. */
inline constexpr float kDefaultMinimumBpm = 80.f;
/** Default upper bound of the tempo range. */
inline constexpr float kDefaultMaximumBpm = 185.f;
/** Lowest selectable tempo bound. */
inline constexpr float kBpmRangeLowerLimit = 30.f;
/** Highest selectable tempo bound. */
inline constexpr float kBpmRangeUpperLimit = 300.f;
/** Capacity of the ring buffer in interleaved samples. */
inline constexpr std::size_t kRingBufferCapacity = std::size_t{1} << 20;
/** Number of interleaved samples the worker consumes per iteration. */
inline constexpr std::size_t kWorkerChunkSamples = 8192;
/** Fallback number of frames for the interleave buffer before the host reports a size. */
inline constexpr std::size_t kFallbackBufferFrames = 2048;

END_NAMESPACE_DISTRHO
