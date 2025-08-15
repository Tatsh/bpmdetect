/** @file */
#pragma once

#include <QSpan>

#pragma clang unsafe_buffer_usage begin

/**
 * Get a QSpan that will not be flagged by `-Wunsafe-buffer-usage`.
 * @param pointer Pointer to the data.
 * @param size Number of elements in the span.
 * @return The constructed QSpan.
 */
template <typename T>
QSpan<T> unsafe_forge_span(T *pointer, qsizetype size) {
    return QSpan(pointer, size);
}

#pragma clang unsafe_buffer_usage end
