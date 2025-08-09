// SPDX-License-Identifier: GPL-3.0-or-later
#include <cstring>
#include <iostream>

#ifdef HAVE_TAGLIB
#include <flacfile.h>
#include <id3v2frame.h>
#include <id3v2tag.h>
#include <textidentificationframe.h>
#include <xiphcomment.h>
#endif // HAVE_TAGLIB

#include "trackflac.h"

using namespace std;
using namespace soundtouch;

TrackFlac::TrackFlac(const char *fname, bool readtags) : Track() {
    setFilename(fname, readtags);
}

TrackFlac::~TrackFlac() {
    close();
}

void TrackFlac::open() {
    close();

    m_iCurPosPCM = 0;
    string fname = filename();

    m_decoder = nullptr;
    m_ibufidx = 0;
    memset(&m_cldata, 0, sizeof(FLAC_CLIENT_DATA));

    FLAC__StreamDecoderInitStatus init_status;

    if ((m_decoder = FLAC__stream_decoder_new()) == NULL) {
        cerr << "TrackFlac: ERROR allocating decoder" << endl;
        return;
    }

    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
    init_status = FLAC__stream_decoder_init_file(
        m_decoder, fname.c_str(), writeCallback, metadataCallback, errorCallback, &m_cldata);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        cerr << "TrackFlac: ERROR initializing decoder"
             << FLAC__StreamDecoderInitStatusString[init_status] << endl;
        return;
    }

    setOpened(true);
    // extract metadata
    FLAC__stream_decoder_process_until_end_of_metadata(m_decoder);

    int channels = m_cldata.channels;
    uint srate = m_cldata.srate;
    unsigned long long numSamples = m_cldata.total_samples;
    uint len = static_cast<uint>(1000ULL * numSamples / srate);

    setLength(len);
    setStartPos(0);
    setEndPos(len);
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
            delete[] m_cldata.buffer;
            m_cldata.buffer = nullptr;
            m_ibufidx = 0;
        }
    }
    m_iCurPosPCM = 0;
    setOpened(false);
}

void TrackFlac::seek(uint ms) {
    if (isValid() && m_decoder) {
        unsigned long long pos =
            (ms * static_cast<unsigned int>(samplerate()) /* * channels()*/) / 1000;
        m_cldata.numsamples = 0;
        if (FLAC__stream_decoder_seek_absolute(m_decoder, pos)) {
            m_ibufidx = 0;
            m_iCurPosPCM = pos;
        } else {
            cerr << "TrackFlac: seek error" << endl;
            if (FLAC__stream_decoder_get_state(m_decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
                FLAC__stream_decoder_flush(m_decoder);
        }
#ifdef DEBUG
    } else {
        cerr << "seek failed: track not valid" << endl;
#endif
    }
}

uint TrackFlac::currentPos() {
    if (isValid()) {
        unsigned long long pos =
            1000 * m_iCurPosPCM / static_cast<unsigned long long>(samplerate() /* *channels()*/);
        return static_cast<uint>(pos);
    }
    return 0;
}

/**
 * Read @a num samples into @a buffer
 * @param buffer pointer to buffer
 * @param num number of samples (per channel)
 * @return number of samples read
 */
int TrackFlac::readSamples(SAMPLETYPE *buffer, unsigned int num) {
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
                buffer[nread++] = (float)m_cldata.buffer[m_ibufidx++] / 32768;
            }
        }

        // do not decode next frame if samples are left (not copied)
        if (m_ibufidx < m_cldata.numsamples)
            break;

        // decode next frame
        if (!FLAC__stream_decoder_process_single(m_decoder)) {
            cerr << "FLAC decode error" << endl;
            break;
        }

        m_ibufidx = 0;
    }

    // return the number of samples in buffer
    m_iCurPosPCM += nread;
    return static_cast<int>(nread);
}

void TrackFlac::storeBPM(string format) {
    string fname = filename();
    string sBPM = bpm2str(getBPM(), format);
#ifdef HAVE_TAGLIB
    TagLib::FLAC::File f(fname.c_str(), false);
    TagLib::Ogg::XiphComment *xiph = f.xiphComment(true);
    if (xiph != NULL) {
        xiph->addField("TBPM", sBPM.c_str(), true); // add new BPM field (replace existing)
    }
    /*
      TagLib::ID3v2::Tag* tag = f.ID3v2Tag (true);
      if (tag != NULL) {
        tag->removeFrames ("TBPM");                 // remove existing BPM frames
        TagLib::ID3v2::TextIdentificationFrame* bpmframe =
          new TagLib::ID3v2::TextIdentificationFrame ("TBPM", TagLib::String::Latin1);
        bpmframe->setText (sBPM.c_str() );
        tag->addFrame (bpmframe);                   // add new BPM frame
      }
    */
    f.save();
#endif
}

void TrackFlac::readTags() {
    string fname = filename();
    string sbpm = "000.00";
#ifdef HAVE_TAGLIB
    TagLib::FLAC::File f(fname.c_str(), false);
    TagLib::Tag *tag = f.tag();
    if (tag != NULL) {
        setArtist(tag->artist().toCString());
        setTitle(tag->title().toCString());
    }

    TagLib::Ogg::XiphComment *xiph = f.xiphComment(true);
    if (xiph != NULL) {
        TagLib::Ogg::FieldListMap flmap = xiph->fieldListMap();
        TagLib::StringList strl = flmap["TBPM"];
        if (!strl.isEmpty())
            sbpm = strl[0].toCString();
        else {
            TagLib::ID3v2::Tag *id3v2tag = f.ID3v2Tag(true);
            if (id3v2tag != NULL) {
                TagLib::List<TagLib::ID3v2::Frame *> lst = id3v2tag->frameList("TBPM");
                if (lst.size() > 0) {
                    TagLib::ID3v2::Frame *frame = lst[0];
                    sbpm = frame->toString().toCString();
                }
            }
        }
    }
#endif
    // set filename (without path) as title if the title is empty
    if (title().empty())
        setTitle(fname.substr(fname.find_last_of("/") + 1));
    setBPM(str2bpm(sbpm));
}

void TrackFlac::removeBPM() {
    string fname = filename();
#ifdef HAVE_TAGLIB
    TagLib::FLAC::File f(fname.c_str(), false);
    TagLib::Ogg::XiphComment *xiph = f.xiphComment(true);
    if (xiph != NULL) {
        xiph->removeFields("TBPM");
    }

    TagLib::ID3v2::Tag *tag = f.ID3v2Tag(true);
    if (tag != NULL) {
        tag->removeFrames("TBPM");
    }

    f.save();
#endif
}

FLAC__StreamDecoderWriteStatus TrackFlac::writeCallback(const FLAC__StreamDecoder *decoder,
                                                        const FLAC__Frame *frame,
                                                        const FLAC__int32 *const buffer[],
                                                        void *client_data) {
    (void)decoder;
    FLAC_CLIENT_DATA *cldata = reinterpret_cast<FLAC_CLIENT_DATA *>(client_data);
    if (!cldata) {
        cerr << "TrackFlac: writeCallback: No client data" << endl;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    // reallocate buffer if required
    if (cldata->buffer != nullptr && frame->header.blocksize * 2 > cldata->bufsize) {
        delete[] cldata->buffer;
        cldata->buffer = nullptr;
    }
    if (cldata->buffer == nullptr) {
        cldata->buffer = new short[frame->header.blocksize * 2];
        cldata->bufsize = frame->header.blocksize * 2;
    }

    cldata->numsamples = frame->header.blocksize * 2;

    // copy samples into the buffer
    for (uint i = 0; i < frame->header.blocksize; ++i) {
        if (cldata->buffer == 0)
            break;

        // 16 bit samples
        if (cldata->bps == 16) {
            cldata->buffer[i * 2] = (FLAC__int16)buffer[0][i];
            if (cldata->channels > 1)
                cldata->buffer[i * 2 + 1] = (FLAC__int16)buffer[1][i];
            // 8 bit samples
        } else if (cldata->bps == 8) {
            cldata->buffer[i * 2] = (FLAC__int16)((FLAC__int8)buffer[0][i]) << 8;
            if (cldata->channels > 1)
                cldata->buffer[i * 2 + 1] = (FLAC__int16)((FLAC__int8)buffer[1][i]) << 8;
            // 24 bit samples
        } else if (cldata->bps == 24) {
            cldata->buffer[i * 2] = (FLAC__int16)((FLAC__int8)buffer[0][i]) >> 8;
            if (cldata->channels > 1)
                cldata->buffer[i * 2 + 1] = (FLAC__int16)((FLAC__int8)buffer[1][i]) >> 8;
            // 32 bit samples
        } else if (cldata->bps == 32) {
            cldata->buffer[i * 2] = (FLAC__int16)((FLAC__int8)buffer[0][i]) >> 16;
            if (cldata->channels > 1)
                cldata->buffer[i * 2 + 1] = (FLAC__int16)((FLAC__int8)buffer[1][i]) >> 16;
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void TrackFlac::metadataCallback(const FLAC__StreamDecoder *decoder,
                                 const FLAC__StreamMetadata *metadata,
                                 void *client_data) {
    (void)decoder;
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
    (void)decoder;
    (void)client_data;
    cerr << "TrackFlac: " << FLAC__StreamDecoderErrorStatusString[status] << endl;
}
