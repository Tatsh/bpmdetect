// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <wavpack/wavpack.h>

#include "track.h"

/** Wavpack file. */
class TrackWavpack : public Track {
public:
    /**
     * Constructor.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackWavpack(const QString &fileName, bool readMetadata = true);
    ~TrackWavpack() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(quint64 ms) override;
    quint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;
    void storeBPM(const QString &sBPM) override;
    void removeBpm() override;

private:
    WavpackContext *wpc = nullptr;
    quint64 m_iCurPosPCM;
};
