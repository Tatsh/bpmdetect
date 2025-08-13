// SPDX-License-Identifier: GPL-3.0-or-later
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QList>
#include <id3v2frame.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <textidentificationframe.h>

#include "trackmp3.h"
#include "utils.h"

TrackMp3::TrackMp3(const QString &fname, bool readMetadata) : Track() {
    setFilename(fname, readMetadata);
}

TrackMp3::~TrackMp3() {
    close();
}

void TrackMp3::clearFrameList() {
    for (qsizetype i = 0; i < m_qSeekList.size(); i++) {
        MadSeekFrameType *p = m_qSeekList.at(i);
        delete p;
        p = nullptr;
    }
    m_qSeekList.clear();
}

void TrackMp3::open() {
    close();

    m_iCurPosPCM = 0;
    rest = 0;
    auto fname = filename();

    // Use QFile to open and read the file
    QFile file(fname);
    setOpened(true);
    if (!file.open(QIODevice::ReadOnly)) {
#ifdef DEBUG
        cerr << "TrackMp3: can not open file" << endl;
#endif
        return;
    }

    // Read the whole file into inputbuf:
    inputbuf = new QByteArray(file.readAll());
    inputbuf_len = inputbuf->size();

    // Transfer it to the mad stream-buffer:
    mad_stream_init(&stream);
    mad_stream_options(&stream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&stream,
                      reinterpret_cast<const unsigned char *>(inputbuf->constData()),
                      static_cast<unsigned long>(inputbuf_len));

    // Decode all the headers, and fill in stats:
    mad_header header;
    currentframe = 0;
    filelength = mad_timer_zero;
    pos = mad_timer_zero;

    int channels = 0;
    uint sRate = 44100;
    clearFrameList();
    while ((stream.bufend - stream.this_frame) > 0) {
        if (mad_header_decode(&header, &stream) == -1) {
            if (!MAD_RECOVERABLE(stream.error)) {
                qCritical() << "mad_header_decode() error:" << mad_stream_errorstr(&stream);
                qCritical() << "Above error is not recoverable.";
                break;
            }
            continue;
        }

        // Add frame to list of frames
        MadSeekFrameType *p = new MadSeekFrameType;
        p->m_pStreamPos = const_cast<unsigned char *>(stream.this_frame);
        p->pos = madLength();
        m_qSeekList.push_back(p);

        currentframe++;
        mad_timer_add(&filelength, header.duration);
        bitrate += header.bitrate;
        sRate = header.samplerate;
        channels = MAD_NCHANNELS(&header);
    }
    // Find average frame size
    if (currentframe)
        m_iAvgFrameSize = static_cast<int>(length() / static_cast<unsigned int>(currentframe));
    else
        m_iAvgFrameSize = 0;

    mad_header_finish(&header);
    if (currentframe == 0)
        bitrate = 0;
    else
        bitrate = bitrate / currentframe;
    framecount = currentframe;
    currentframe = 0;

    frame = new mad_frame;
    // Re-init buffer:
    setValid(true);
    seek(0);

    setSampleRate(static_cast<int>(sRate));
    unsigned long long numSamples =
        static_cast<unsigned long long>(madLength()) / static_cast<unsigned long long>(channels);
    uint len = static_cast<uint>(1000 * numSamples / sRate);

    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSampleBytes(2);
    setChannels(channels);
    setTrackType(TYPE_MPEG);
}

void TrackMp3::close() {
    if (!isOpened())
        return;
    m_iCurPosPCM = 0;
    clearFrameList();
    delete inputbuf;
    inputbuf_len = 0;
    inputbuf = nullptr;
    setOpened(false);
}

inline long TrackMp3::madLength() {
    enum mad_units units;

    int sRate = sampleRate();
    switch (sRate) {
    case 8000:
        units = MAD_UNITS_8000_HZ;
        break;
    case 11025:
        units = MAD_UNITS_11025_HZ;
        break;
    case 12000:
        units = MAD_UNITS_12000_HZ;
        break;
    case 16000:
        units = MAD_UNITS_16000_HZ;
        break;
    case 22050:
        units = MAD_UNITS_22050_HZ;
        break;
    case 24000:
        units = MAD_UNITS_24000_HZ;
        break;
    case 32000:
        units = MAD_UNITS_32000_HZ;
        break;
    case 44100:
        units = MAD_UNITS_44100_HZ;
        break;
    case 48000:
        units = MAD_UNITS_48000_HZ;
        break;
    default: // By the MP3 specs, an MP3 _has_ to have one of the above sample rates...
        units = MAD_UNITS_44100_HZ;
        setSampleRate(44100);
    }

    return 2 * mad_timer_count(filelength, units);
}

void TrackMp3::seek(qint64 ms) {
    if (!isValid()) {
#ifdef DEBUG
        cerr << "seek failed: track not valid" << endl;
#endif
        return;
    }
    auto seek_pos = (ms * sampleRate() /* * channels()*/) / 1000;

    // Ensure that we are seeking to an even pos
    //Q_ASSERT(pos%2==0);
    MadSeekFrameType *cur;

    if (seek_pos == 0) {
        // Seek to beginning of file
        // Re-init buffer:
        mad_stream_finish(&stream);
        mad_stream_init(&stream);
        mad_stream_options(&stream, MAD_OPTION_IGNORECRC);
        mad_stream_buffer(&stream,
                          reinterpret_cast<const unsigned char *>(inputbuf->constData()),
                          static_cast<unsigned long>(inputbuf_len));
        mad_frame_init(frame);
        mad_synth_init(&synth);
        rest = -1;
        cur = m_qSeekList.at(0);
    } else {
        // Perform precise seek accomplished by using a frame in the seek list
        // Find the frame to seek to in the list
        auto framePos = findFrame(static_cast<int>(seek_pos));

        uint frameIdx = 10; //m_qSeekList.at();

        if (framePos == 0 || framePos > seek_pos || frameIdx < 5) {
            // Re-init buffer:
            mad_stream_finish(&stream);
            mad_stream_init(&stream);
            mad_stream_options(&stream, MAD_OPTION_IGNORECRC);
            mad_stream_buffer(&stream,
                              reinterpret_cast<const unsigned char *>(inputbuf->constData()),
                              static_cast<unsigned long>(inputbuf_len));
            mad_frame_init(frame);
            mad_synth_init(&synth);
            rest = -1;
            cur = m_qSeekList.at(0);
        } else {
            // Start four frame before wanted frame to get in sync...
            cur = m_qSeekList[frameIdx - 4];

            // Start from the new frame
            mad_stream_finish(&stream);
            mad_stream_init(&stream);
            mad_stream_options(&stream, MAD_OPTION_IGNORECRC);
            auto length = static_cast<unsigned long>(
                inputbuf_len -
                static_cast<long int>(cur->m_pStreamPos - reinterpret_cast<const unsigned char *>(
                                                              inputbuf->constData())));
            mad_stream_buffer(
                &stream, static_cast<const unsigned char *>(cur->m_pStreamPos), length);
            mad_synth_mute(&synth);
            mad_frame_mute(frame);

            // Decode the three frames before
            mad_frame_decode(frame, &stream);
            mad_frame_decode(frame, &stream);
            mad_frame_decode(frame, &stream);
            if (mad_frame_decode(frame, &stream))
                qDebug() << "MP3 decode warning.";
            mad_synth_frame(&synth, frame);

            // Set current position
            rest = -1;
            cur = m_qSeekList[frameIdx];
        }

        // synthesize the samples from the frame which should be discard to reach the requested position
        discard(static_cast<unsigned long>(seek_pos - cur->pos));
    }

    // Unfortunately we don't know the exact position. The returned position is thus an
    // approximation only:
    m_iCurPosPCM = seek_pos;
}

qint64 TrackMp3::currentPos() {
    if (isValid()) {
        return 1000 * m_iCurPosPCM / sampleRate() /* *channels()*/;
    }
    return 0;
}

int TrackMp3::readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) {
    auto num = buffer.size();
    if (!isValid() || num < 2)
        return -1;

    // Ensure that we are reading an even number of samples. Otherwise this function may
    // go into an infinite loop
    if (num % 2 != 0)
        num--;
    int nchannels = channels();
    unsigned nSamples = 0;
    QList<short> dest(static_cast<int>(num));
    qsizetype dest_index = 0;

    // If samples are left from previous read, then copy them to start of destination
    if (rest > 0) {
        auto leftSpan = unsafe_forge_span(synth.pcm.samples[0], synth.pcm.length);
        auto rightSpan = nchannels > 1 ? unsafe_forge_span(synth.pcm.samples[1], synth.pcm.length) :
                                         QSpan<mad_fixed_t>();
        for (int i = rest; i < synth.pcm.length; i++) {
            // Left channel
            if (dest_index < dest.size())
                dest[dest_index++] = static_cast<short>(madScale(leftSpan[i]));
            // Right channel
            if (nchannels > 1 && dest_index < dest.size())
                dest[dest_index++] = static_cast<short>(madScale(rightSpan[i]));
        }
        nSamples += static_cast<unsigned int>(nchannels) *
                    (static_cast<unsigned int>(synth.pcm.length) - static_cast<unsigned int>(rest));
    }

    int no = 0;
    while (nSamples < num) {
        if (mad_frame_decode(frame, &stream)) {
            if (MAD_RECOVERABLE(stream.error)) {
#ifdef DEBUG
                cerr << "MAD: Recoverable frame level ERR (" << mad_stream_errorstr(&stream) << ")"
                     << endl;
#endif
                continue;
            } else if (stream.error == MAD_ERROR_BUFLEN) {
#ifdef DEBUG
                cerr << "MAD: buflen ERR" << endl;
#endif
                break;
            } else {
#ifdef DEBUG
                cerr << "MAD: Unrecoverable frame level ERR (" << mad_stream_errorstr(&stream)
                     << ")";
#endif
                break;
            }
        }

        /* Once decoded the frame is synthesized to PCM samples. No ERRs
         * are reported by mad_synth_frame();
         */
        mad_synth_frame(&synth, frame);

        // Number of channels in frame
        // ch = MAD_NCHANNELS(&frame->header);

        /* Synthesized samples must be converted from mad's fixed
         * point number to the consumer format (16 bit). Integer samples
         * are temporarily stored in a buffer that is flushed when
         * full.
         */
        no = static_cast<int>(qMin(static_cast<long long>(synth.pcm.length), (num - nSamples) / 2));
        auto leftSpan = unsafe_forge_span(synth.pcm.samples[0], no);
        auto rightSpan =
            nchannels > 1 ? unsafe_forge_span(synth.pcm.samples[1], no) : QSpan<mad_fixed_t>();
        for (int i = 0; i < no; i++) {
            // Left channel
            if (dest_index < dest.size())
                dest[dest_index++] = static_cast<short>(madScale(leftSpan[i]));

            // Right channel
            if (nchannels > 1 && dest_index < dest.size())
                dest[dest_index++] = static_cast<short>(madScale(rightSpan[i]));
        }
        nSamples += static_cast<unsigned int>(nchannels) * static_cast<unsigned int>(no);
    }

    // If samples are still left in buffer, set rest to the index of the unused samples
    if (synth.pcm.length > no)
        rest = no;
    else
        rest = -1;

    // convert the samples to float
    for (unsigned int i = 0; i < nSamples; ++i) {
        buffer[i] = static_cast<float>(dest[i]) / SAMPLE_MAX_VALUE;
    }

    m_iCurPosPCM += nSamples;
    return static_cast<int>(nSamples);
}

inline signed int TrackMp3::madScale(mad_fixed_t sample) {
    sample += (1L << (MAD_F_FRACBITS - 16));

    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

// Decode the chosen number of samples and discard
unsigned long TrackMp3::discard(unsigned long samples_wanted) {
    unsigned long Total_samples_decoded = 0;
    int no = 0;

    if (rest > 0)
        Total_samples_decoded +=
            2 * (static_cast<unsigned long>(synth.pcm.length) - static_cast<unsigned long>(rest));

    while (Total_samples_decoded < samples_wanted) {
        if (mad_frame_decode(frame, &stream)) {
            if (MAD_RECOVERABLE(stream.error)) {
                continue;
            } else if (stream.error == MAD_ERROR_BUFLEN) {
                break;
            } else {
                break;
            }
        }
        mad_synth_frame(&synth, frame);
        no = static_cast<int>(qMin(static_cast<unsigned long>(synth.pcm.length),
                                   (samples_wanted - Total_samples_decoded) / 2));
        Total_samples_decoded += 2 * static_cast<unsigned long>(no);
    }

    if (synth.pcm.length > no)
        rest = no;
    else
        rest = -1;

    return Total_samples_decoded;
}

int TrackMp3::findFrame(int framePos) {
    // Guess position of frame in m_qSeekList based on average frame size
    uint frameIdx = qMin(static_cast<uint>(m_qSeekList.size() - 1),
                         m_iAvgFrameSize ? static_cast<uint>(framePos / m_iAvgFrameSize) : 0);
    MadSeekFrameType *temp = m_qSeekList.at(frameIdx);

    // Ensure that the list element is not at a greater position than framePos
    while (temp != nullptr && temp->pos > framePos) {
        temp = m_qSeekList.at(--frameIdx);
    }

    // Ensure that the following position is also not smaller than framePos
    if (temp != nullptr) {
        while (temp != nullptr && temp->pos < framePos) {
            temp = m_qSeekList.at(++frameIdx);
        }

        if (temp == nullptr)
            temp = m_qSeekList.back();
        else
            temp = m_qSeekList.at(--frameIdx);
    }

    if (temp) {
        return static_cast<int>(temp->pos);
    }
    return 0;
}

void TrackMp3::storeBPM(const QString &format) {
    auto fname = filename();
    auto sBPM = bpm2str(getBPM(), format);
    TagLib::MPEG::File f(fname.toUtf8().constData(), false);
    auto tag = f.ID3v2Tag(true);
    if (tag == nullptr) {
        qCritical() << "BPM not saved.";
        return;
    }
    tag->removeFrames("TBPM"); // remove existing BPM frames
    auto bpmFrame = new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
    bpmFrame->setText(TagLib::String(sBPM.toStdString()));
    tag->addFrame(bpmFrame); // add new BPM frame
    f.save();                // save file
}

void TrackMp3::readTags() {
    auto fname = filename();
    auto sBPM = QStringLiteral("000.00");
    TagLib::MPEG::File f(fname.toUtf8().constData(), false);

    TagLib::ID3v2::Tag *tag = f.ID3v2Tag(false);
    if (tag != NULL) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));

        auto lst = tag->frameList("TBPM");
        if (lst.size() > 0) {
            TagLib::ID3v2::Frame *frame2 = lst[0];
            sBPM = QString::fromUtf8(frame2->toString().toCString(true));
        }
    }
    // set filename (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBPM(str2bpm(sBPM));
}

void TrackMp3::removeBPM() {
    auto fname = filename();
    TagLib::MPEG::File f(fname.toUtf8().constData(), false);
    TagLib::ID3v2::Tag *tag = f.ID3v2Tag(true);
    if (tag == nullptr) {
        return;
    }
    tag->removeFrames("TBPM");
    f.save();
}
