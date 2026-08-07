// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <bit>

#include "sampleringbuffer.h"

SampleRingBuffer::SampleRingBuffer(std::size_t capacity)
    : storage_(std::bit_ceil(capacity)), mask_(storage_.size() - 1) {
}

std::size_t SampleRingBuffer::write(std::span<const float> samples) {
    const auto writePosition = writePosition_.load(std::memory_order_relaxed);
    const auto readPosition = readPosition_.load(std::memory_order_acquire);
    const auto available = storage_.size() - (writePosition - readPosition);
    if (samples.size() > available) {
        return 0;
    }
    for (std::size_t i = 0; i < samples.size(); ++i) {
        storage_[(writePosition + i) & mask_] = samples[i];
    }
    writePosition_.store(writePosition + samples.size(), std::memory_order_release);
    return samples.size();
}

std::size_t SampleRingBuffer::read(std::span<float> destination) {
    const auto readPosition = readPosition_.load(std::memory_order_relaxed);
    const auto writePosition = writePosition_.load(std::memory_order_acquire);
    const auto available = writePosition - readPosition;
    const auto count = std::min(destination.size(), available);
    for (std::size_t i = 0; i < count; ++i) {
        destination[i] = storage_[(readPosition + i) & mask_];
    }
    readPosition_.store(readPosition + count, std::memory_order_release);
    return count;
}

void SampleRingBuffer::discardAll() {
    readPosition_.store(writePosition_.load(std::memory_order_acquire), std::memory_order_release);
}
