// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QSpan>
#include <QtCore/QString>

#pragma clang unsafe_buffer_usage begin
/**
 * Get a QSpan that will not be flagged by `-Wunsafe-buffer-usage`.
 * @param pointer Pointer to the data.
 * @param size Number of elements in the span.
 * @return The constructed QSpan.
 */
template <typename T>
QSpan<T> unsafeSpan(T *pointer, qsizetype size) {
    return QSpan(pointer, size);
}
#pragma clang unsafe_buffer_usage end

/** BPM type. */
typedef double bpmtype;

/**
 * Convert QString to BPM.
 * @param sBpm BPM string.
 * @return BPM value.
 */
bpmtype stringToBpm(const QString &sBpm);

/**
 * Convert BPM to QString using selected format.
 * @param dBpm BPM value.
 * @param format Format (default `"0.00"`, other possible values: `"0.0"`, `"0"`, `"000.00"`,
 * `"000.0"`, `"000"`, `"00000"`).
 * @return Formatted BPM string.
 */
QString bpmToString(bpmtype dBpm, const QString &format = QStringLiteral("0.00"));
