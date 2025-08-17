// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <mad.h>

#include "track.h"

/** Struct used to store mad frames for seeking. */
typedef struct MadSeekFrameType {
    unsigned char *m_pStreamPos; //!< Stream position.
    unsigned long int pos;       //!< Position in PCM samples.
} MadSeekFrameType;

/** MP3 file. */
class TrackMp3 : public Track {
public:
    /** Constructor.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackMp3(const QString &fileName, bool readMetadata = true);
    ~TrackMp3() override;
    void readTags() override;

protected:
    void open() override;
    void close() override;
    void seek(quint64 ms) override;
    quint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) override;
    void storeBpm(const QString &sBpm) override;
    void removeBpm() override;

private:
    unsigned long discard(unsigned long samples_wanted);
    unsigned long findFrame(unsigned long pos);
    void clearFrameList();
    inline long madLength();
    inline signed int madScale(mad_fixed_t sample);

    // It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
    // To have precise seek within a limited range from the current decode position, we keep track
    // of past decodeded frame, and their exact position. If a seek occurs and it is within the
    // range of frames we keep track of a precise seek occurs, otherwise an imprecise seek is
    // performed.
    QList<MadSeekFrameType *> m_qSeekList;
    QByteArray *inputbuf;
    qsizetype inputbuf_len;
    unsigned int framecount;
    unsigned int currentframe;
    unsigned int bitrate;
    int rest;
    quint64 m_iAvgFrameSize;
    quint64 m_iCurPosPCM;
    mad_timer_t pos;
    mad_timer_t filelength;
    mad_stream stream;
    mad_frame *frame;
    mad_synth synth;
};
