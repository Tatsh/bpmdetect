// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDebug>
#include <flacfile.h>
#include <id3v2frame.h>
#include <id3v2tag.h>
#include <textidentificationframe.h>
#include <xiphcomment.h>

#include "trackflac.h"

TrackFlac::TrackFlac(const QString &fname, bool readMetadata) : Track() {
    setFileName(fname, readMetadata);
}

TrackFlac::~TrackFlac() {
    close();
}

void TrackFlac::open() {
    close();

    m_iCurPosPCM = 0;
    auto fname = fileName();

    m_decoder = nullptr;
    m_ibufidx = 0;

    FLAC__StreamDecoderInitStatus init_status;

    if ((m_decoder = FLAC__stream_decoder_new()) == nullptr) {
        qCritical() << "Error allocating decoder.";
        return;
    }

    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
    init_status = FLAC__stream_decoder_init_file(m_decoder,
                                                 fname.toUtf8().constData(),
                                                 writeCallback,
                                                 metadataCallback,
                                                 errorCallback,
                                                 &m_clData);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qCritical() << "Error initialising decoder:"
                    << FLAC__StreamDecoderInitStatusString[init_status];
        return;
    }

    setOpened(true);
    // extract metadata
    FLAC__stream_decoder_process_until_end_of_metadata(m_decoder);

    auto channels = m_clData.channels;
    auto sRate = m_clData.sRate;
    auto numSamples = m_clData.total_samples;
    auto len = 1000 * numSamples / sRate;

    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSampleRate(sRate);
    setSampleBytes(2);
    setChannels(static_cast<unsigned int>(channels));
    setTrackType(Flac);
    setValid(true);
}

void TrackFlac::close() {
    if (isOpened()) {
        if (m_decoder != nullptr) {
            FLAC__stream_decoder_finish(m_decoder);
            FLAC__stream_decoder_delete(m_decoder);
            m_decoder = nullptr;
        }
        if (m_clData.buffer != nullptr) {
            delete m_clData.buffer;
            m_clData.buffer = nullptr;
            m_ibufidx = 0;
        }
    }
    m_iCurPosPCM = 0;
    setOpened(false);
}

void TrackFlac::seek(quint64 ms) {
    if (isValid() && m_decoder) {
        auto pos = (ms * static_cast<unsigned int>(sampleRate()) /* * channels()*/) / 1000;
        m_clData.numSamples = 0;
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

quint64 TrackFlac::currentPos() {
    if (isValid()) {
        auto pos = (1000 * m_iCurPosPCM) / sampleRate() /* *channels()*/;
        return pos;
    }
    return 0;
}

int TrackFlac::readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) {
    auto num = buffer.size();
    if (!isValid() || !m_decoder || num < 2)
        return -1;

    auto state = FLAC__stream_decoder_get_state(m_decoder);
    if (state == FLAC__STREAM_DECODER_END_OF_STREAM || state == FLAC__STREAM_DECODER_ABORTED) {
        return 0;
    }

    unsigned int nread = 0;

    while (nread < num) {
        if (m_clData.buffer) {
            // copy samples to destination
            while (m_ibufidx < m_clData.numSamples && nread < num) {
                buffer[nread++] =
                    static_cast<float>((*m_clData.buffer)[m_ibufidx++]) / SAMPLE_MAX_VALUE;
            }
        }

        // do not decode next frame if samples are left (not copied)
        if (m_ibufidx < m_clData.numSamples)
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
    auto fname = fileName();
    auto sBPM = bpmToString(bpm(), format);
    TagLib::FLAC::File f(fname.toUtf8().constData(), false);
    auto xiph = f.xiphComment(true);
    if (xiph != nullptr) {
        xiph->addField(
            "TBPM", sBPM.toUtf8().constData(), true); // add new BPM field (replace existing)
    }
    f.save();
}

void TrackFlac::readTags() {
    auto fname = fileName();
    auto sBPM = QStringLiteral("000.00");
    TagLib::FLAC::File f(fname.toUtf8().constData(), false);
    auto tag = f.tag();
    if (tag != nullptr) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));
    }

    TagLib::Ogg::XiphComment *xiph = f.xiphComment(true);
    if (xiph != nullptr) {
        auto flMap = xiph->fieldListMap();
        auto strl = flMap["TBPM"];
        if (!strl.isEmpty())
            sBPM = QString::fromUtf8(strl[0].toCString(true));
        else {
            auto id3v2tag = f.ID3v2Tag(true);
            if (id3v2tag != nullptr) {
                auto lst = id3v2tag->frameList("TBPM");
                if (lst.size() > 0) {
                    auto frame = lst[0];
                    sBPM = QString::fromUtf8(frame->toString().toCString(true));
                }
            }
        }
    }
    // set fileName (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBpm(stringToBpm(sBPM));
}

void TrackFlac::removeBpm() {
    auto fname = fileName();
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
    auto clientData = reinterpret_cast<FLAC_CLIENT_DATA *>(client_data);
    if (!clientData) {
        qCritical() << "No client data.";
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    // reallocate buffer if required
    if (clientData->buffer != nullptr && frame->header.blocksize * 2 > clientData->bufsize) {
        qDebug() << "Reallocated buffer for FLAC samples.";
        delete clientData->buffer;
        clientData->buffer = nullptr;
    }
    if (clientData->buffer == nullptr) {
        clientData->buffer = new QList<short>(frame->header.blocksize * 2);
        clientData->bufsize = frame->header.blocksize * 2;
    }

    clientData->numSamples = frame->header.blocksize * 2;
    auto leftBuffSpan = unsafe_forge_span(buffer[0], frame->header.blocksize);
    auto rightBuffSpan = clientData->channels > 1 ?
                             unsafe_forge_span(buffer[1], frame->header.blocksize) :
                             QSpan<FLAC__int32>();

    // copy samples into the buffer
    for (uint32_t i = 0; i < frame->header.blocksize; ++i) {
        // 16 bit samples
        if (clientData->bps == 16) {
            (*clientData->buffer)[i * 2] = static_cast<FLAC__int16>(leftBuffSpan[i]);
            if (clientData->channels > 1)
                (*clientData->buffer)[i * 2 + 1] = static_cast<FLAC__int16>(rightBuffSpan[i]);
            // 8 bit samples
        } else if (clientData->bps == 8) {
            (*clientData->buffer)[i * 2] =
                static_cast<FLAC__int16>(static_cast<FLAC__int8>(leftBuffSpan[i]) << 8);
            if (clientData->channels > 1)
                (*clientData->buffer)[i * 2 + 1] =
                    static_cast<FLAC__int16>(static_cast<FLAC__int8>(rightBuffSpan[i]) << 8);
            // 24 bit samples
        } else if (clientData->bps == 24) {
            (*clientData->buffer)[i * 2] =
                static_cast<FLAC__int16>(static_cast<FLAC__int8>(leftBuffSpan[i])) >> 8;
            if (clientData->channels > 1)
                (*clientData->buffer)[i * 2 + 1] =
                    static_cast<FLAC__int16>(static_cast<FLAC__int8>(rightBuffSpan[i])) >> 8;
            // 32 bit samples
        } else if (clientData->bps == 32) {
            (*clientData->buffer)[i * 2] =
                static_cast<FLAC__int16>(static_cast<FLAC__int8>(leftBuffSpan[i])) >> 16;
            if (clientData->channels > 1)
                (*clientData->buffer)[i * 2 + 1] =
                    static_cast<FLAC__int16>(static_cast<FLAC__int8>(rightBuffSpan[i])) >> 16;
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void TrackFlac::metadataCallback(const FLAC__StreamDecoder *decoder,
                                 const FLAC__StreamMetadata *metadata,
                                 void *client_data) {
    Q_UNUSED(decoder)
    auto info = reinterpret_cast<FLAC_CLIENT_DATA *>(client_data);

    if (info != nullptr && metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        info->sRate = metadata->data.stream_info.sample_rate;
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
