// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once
#include <atomic>
#include <cstddef>
#include <span>
#include <vector>

/**
 * Lock-free single-producer, single-consumer ring buffer for audio samples.
 *
 * The producer (real-time audio thread) only calls write(), and the consumer (worker thread) only
 * calls read() and discardAll(). Writes are all-or-nothing so interleaved frames are never split.
 */
class SampleRingBuffer {
public:
    /** Constructs a ring buffer holding at least @a capacity samples (rounded up to a power of
     * two). */
    explicit SampleRingBuffer(std::size_t capacity);
    /** Writes all samples or none if there is not enough space. Returns the number of samples
     * written. Producer only. Real-time safe. */
    std::size_t write(std::span<const float> samples);
    /** Reads up to destination.size() samples. Returns the number of samples read. Consumer
     * only. */
    std::size_t read(std::span<float> destination);
    /** Discards all buffered samples. Consumer only. */
    void discardAll();

private:
    std::vector<float> storage_;
    std::size_t mask_ = 0;
    std::atomic<std::size_t> readPosition_{0};
    std::atomic<std::size_t> writePosition_{0};
};
