// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QFile>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "track.h"

class TrackOggVorbis : public Track {
public:
    TrackOggVorbis(const QString &filename, bool readtags = true);
    ~TrackOggVorbis() override;
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
    qint64 m_iCurPosPCM;
    QFile fptr;
    OggVorbis_File vf;
    int current_section;
};
