// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <wavpack/wavpack.h>

#include "track.h"

/** Wavpack file. */
class TrackWavpack : public Track {
public:
    /**
     * Constructor.
     * @param filename Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackWavpack(const QString &filename, bool readMetadata = true);
    ~TrackWavpack() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;
    void storeBPM(const QString &sBPM) override;
    void removeBPM() override;

private:
    WavpackContext *wpc = nullptr;
    qint64 m_iCurPosPCM;
};
