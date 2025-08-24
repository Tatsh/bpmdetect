// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QMap>
#include <QtCore/QVariant>

class QString;

/** Check if a file can be decoded using ffmpeg. */
bool isDecodableFile(const QString &file);

/**
 * Store BPM to audio file.
 *
 * If this is a MP3 file, the BPM will be stored in the TBPM tag of ID3v2. Other formats will
 * have the BPM saved in a generic "BPM" tag. This will return `false` in those cases as well as for
 * any errors.
 *
 * @param fileName The path to the audio file.
 * @param sBpm The BPM value to store.
 * @return `true` if the BPM was successfully stored, `false` otherwise.
 */
bool storeBpmInFile(const QString &fileName, const QString &sBpm);

/**
 * Remove BPM metadata from audio file.
 *
 * If this is a MP3 file, the TBPM tag of ID3v2 will be removed. Other formats will have the
 * generic "BPM" tag removed.
 *
 * @param fileName The path to the audio file.
 * @return `true` if the BPM metadata was successfully removed, `false` otherwise.
 */
bool removeBpmFromFile(const QString &fileName);

/**
 * Tag reader using ffmpeg. Gets artist, title, bpm and length (in milliseconds) and puts them in a
 * map.
 * @param fileName_ The path to the audio file.
 * @return A map with the tags read. If a tag is not found, the key will have a sane default value.
 */
QMap<QString, QVariant> readTagsFromFile(const QString &fileName);
