// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDebug>
#include <flacfile.h>
#include <id3v2frame.h>
#include <id3v2tag.h>
#include <textidentificationframe.h>
#include <xiphcomment.h>

#include "trackflac.h"
#include "utils.h"

TrackFlac::TrackFlac(const QString &fname, bool readMetadata) : Track() {
    setFilename(fname, readMetadata);
}

TrackFlac::~TrackFlac() {
    close();
}

void TrackFlac::open() {
    close();

    m_iCurPosPCM = 0;
    auto fname = filename();

    m_decoder = nullptr;
    m_ibufidx = 0;

    FLAC__StreamDecoderInitStatus init_status;

    if ((m_decoder = FLAC__stream_decoder_new()) == NULL) {
        qCritical() << "Error allocating decoder.";
        return;
    }

    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
    init_status = FLAC__stream_decoder_init_file(m_decoder,
                                                 fname.toUtf8().constData(),
                                                 writeCallback,
                                                 metadataCallback,
                                                 errorCallback,
                                                 &m_cldata);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qCritical() << "Error initialising decoder:"
                    << FLAC__StreamDecoderInitStatusString[init_status];
        return;
    }

    setOpened(true);
    // extract metadata
    FLAC__stream_decoder_process_until_end_of_metadata(m_decoder);

    auto channels = m_cldata.channels;
    auto srate = m_cldata.srate;
    auto numSamples = m_cldata.total_samples;
    auto len = 1000 * numSamples / srate;

    setLength(static_cast<unsigned int>(len));
    setStartPos(0);
    setEndPos(static_cast<qint64>(len));
    setSamplerate(static_cast<int>(srate));
    setSampleBytes(2);
    setChannels(channels);
    setTrackType(TYPE_FLAC);
    setValid(true);
}

void TrackFlac::close() {
    if (isOpened()) {
        if (m_decoder != nullptr) {
            FLAC__stream_decoder_finish(m_decoder);
            FLAC__stream_decoder_delete(m_decoder);
            m_decoder = nullptr;
        }
        if (m_cldata.buffer != nullptr) {
            delete m_cldata.buffer;
            m_cldata.buffer = nullptr;
            m_ibufidx = 0;
        }
    }
    m_iCurPosPCM = 0;
    setOpened(false);
}

void TrackFlac::seek(qint64 ms) {
    if (isValid() && m_decoder) {
        auto pos = (ms * static_cast<unsigned int>(samplerate()) /* * channels()*/) / 1000;
        m_cldata.numsamples = 0;
        if (FLAC__stream_decoder_seek_absolute(m_decoder, static_cast<FLAC__uint64>(pos))) {
            m_ibufidx = 0;
            m_iCurPosPCM = pos;
        } else {
            qCritical() << "Seek error";
            if (FLAC__stream_decoder_get_state(m_decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
                FLAC__stream_decoder_flush(m_decoder);
        }
    }
}

qint64 TrackFlac::currentPos() {
    if (isValid()) {
        auto pos = (1000 * m_iCurPosPCM) / samplerate() /* *channels()*/;
        return pos;
    }
    return 0;
}

int TrackFlac::readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) {
    auto num = buffer.size();
    if (!isValid() || !m_decoder || num < 2)
        return -1;

    FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(m_decoder);
    if (state == FLAC__STREAM_DECODER_END_OF_STREAM || state == FLAC__STREAM_DECODER_ABORTED) {
        return 0;
    }

    unsigned int nread = 0;

    while (nread < num) {
        if (m_cldata.buffer) {
            // copy samples to destination
            while (m_ibufidx < m_cldata.numsamples && nread < num) {
                buffer[nread++] =
                    static_cast<float>((*m_cldata.buffer)[m_ibufidx++]) / SAMPLE_MAXVALUE;
            }
        }

        // do not decode next frame if samples are left (not copied)
        if (m_ibufidx < m_cldata.numsamples)
            break;

        // decode next frame
        if (!FLAC__stream_decoder_process_single(m_decoder)) {
            qCritical() << "FLAC decode error.";
            break;
        }

        m_ibufidx = 0;
    }

    // return the number of samples in buffer
    m_iCurPosPCM += nread;
    return static_cast<int>(nread);
}

void TrackFlac::storeBPM(const QString &format) {
    auto fname = filename();
    auto sBPM = bpm2str(getBPM(), format);
    TagLib::FLAC::File f(fname.toUtf8().constData(), false);
    auto xiph = f.xiphComment(true);
    if (xiph != NULL) {
        xiph->addField(
            "TBPM", sBPM.toUtf8().constData(), true); // add new BPM field (replace existing)
    }
    f.save();
}

void TrackFlac::readTags() {
    auto fname = filename();
    auto sbpm = QStringLiteral("000.00");
    TagLib::FLAC::File f(fname.toUtf8().constData(), false);
    TagLib::Tag *tag = f.tag();
    if (tag != NULL) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));
    }

    TagLib::Ogg::XiphComment *xiph = f.xiphComment(true);
    if (xiph != NULL) {
        TagLib::Ogg::FieldListMap flmap = xiph->fieldListMap();
        TagLib::StringList strl = flmap["TBPM"];
        if (!strl.isEmpty())
            sbpm = QString::fromUtf8(strl[0].toCString(true));
        else {
            TagLib::ID3v2::Tag *id3v2tag = f.ID3v2Tag(true);
            if (id3v2tag != NULL) {
                TagLib::List<TagLib::ID3v2::Frame *> lst = id3v2tag->frameList("TBPM");
                if (lst.size() > 0) {
                    TagLib::ID3v2::Frame *frame = lst[0];
                    sbpm = QString::fromUtf8(frame->toString().toCString(true));
                }
            }
        }
    }
    // set filename (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBPM(str2bpm(sbpm));
}

void TrackFlac::removeBPM() {
    auto fname = filename();
    TagLib::FLAC::File f(fname.toUtf8().constData(), false);
    auto xiph = f.xiphComment(true);
    if (xiph != nullptr) {
        xiph->removeFields("TBPM");
    }

    auto tag = f.ID3v2Tag(true);
    if (tag != nullptr) {
        tag->removeFrames("TBPM");
    }

    f.save();
}

FLAC__StreamDecoderWriteStatus TrackFlac::writeCallback(const FLAC__StreamDecoder *decoder,
                                                        const FLAC__Frame *frame,
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
                                                        const FLAC__int32 *const buffer[],
#pragma clang diagnostic pop
                                                        void *client_data) {
    (void)decoder;
    FLAC_CLIENT_DATA *cldata = reinterpret_cast<FLAC_CLIENT_DATA *>(client_data);
    if (!cldata) {
        qCritical() << "No client data.";
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    // reallocate buffer if required
    if (cldata->buffer != nullptr && frame->header.blocksize * 2 > cldata->bufsize) {
        qDebug() << "Reallocated buffer for FLAC samples.";
        delete cldata->buffer;
        cldata->buffer = nullptr;
    }
    if (cldata->buffer == nullptr) {
        cldata->buffer = new QList<short>(frame->header.blocksize * 2);
        cldata->bufsize = frame->header.blocksize * 2;
    }

    cldata->numsamples = frame->header.blocksize * 2;
    auto leftBuffSpan = unsafe_forge_span(buffer[0], frame->header.blocksize);
    auto rightBuffSpan = cldata->channels > 1 ?
                             unsafe_forge_span(buffer[1], frame->header.blocksize) :
                             QSpan<FLAC__int32>();

    // copy samples into the buffer
    for (uint i = 0; i < frame->header.blocksize; ++i) {
        // 16 bit samples
        if (cldata->bps == 16) {
            (*cldata->buffer)[i * 2] = static_cast<FLAC__int16>(leftBuffSpan[i]);
            if (cldata->channels > 1)
                (*cldata->buffer)[i * 2 + 1] = static_cast<FLAC__int16>(rightBuffSpan[i]);
            // 8 bit samples
        } else if (cldata->bps == 8) {
            (*cldata->buffer)[i * 2] =
                static_cast<FLAC__int16>(static_cast<FLAC__int8>(leftBuffSpan[i]) << 8);
            if (cldata->channels > 1)
                (*cldata->buffer)[i * 2 + 1] =
                    static_cast<FLAC__int16>(static_cast<FLAC__int8>(rightBuffSpan[i]) << 8);
            // 24 bit samples
        } else if (cldata->bps == 24) {
            (*cldata->buffer)[i * 2] =
                static_cast<FLAC__int16>(static_cast<FLAC__int8>(leftBuffSpan[i])) >> 8;
            if (cldata->channels > 1)
                (*cldata->buffer)[i * 2 + 1] =
                    static_cast<FLAC__int16>(static_cast<FLAC__int8>(rightBuffSpan[i])) >> 8;
            // 32 bit samples
        } else if (cldata->bps == 32) {
            (*cldata->buffer)[i * 2] =
                static_cast<FLAC__int16>(static_cast<FLAC__int8>(leftBuffSpan[i])) >> 16;
            if (cldata->channels > 1)
                (*cldata->buffer)[i * 2 + 1] =
                    static_cast<FLAC__int16>(static_cast<FLAC__int8>(rightBuffSpan[i])) >> 16;
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void TrackFlac::metadataCallback(const FLAC__StreamDecoder *decoder,
                                 const FLAC__StreamMetadata *metadata,
                                 void *client_data) {
    Q_UNUSED(decoder)
    FLAC_CLIENT_DATA *info = reinterpret_cast<FLAC_CLIENT_DATA *>(client_data);

    if (info != nullptr && metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        info->srate = metadata->data.stream_info.sample_rate;
        info->channels = static_cast<int>(metadata->data.stream_info.channels);
        info->total_samples = metadata->data.stream_info.total_samples;
        info->bps = metadata->data.stream_info.bits_per_sample;
    }
}

void TrackFlac::errorCallback(const FLAC__StreamDecoder *decoder,
                              FLAC__StreamDecoderErrorStatus status,
                              void *client_data) {
    Q_UNUSED(decoder)
    Q_UNUSED(client_data)
    qCritical() << FLAC__StreamDecoderErrorStatusString[status];
}
