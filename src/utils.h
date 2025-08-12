#pragma once

#include <QSpan>

#pragma clang unsafe_buffer_usage begin

template <typename T>
QSpan<T> unsafe_forge_span(T *pointer, qsizetype size) {
    return QSpan(pointer, size);
}

#pragma clang unsafe_buffer_usage end
