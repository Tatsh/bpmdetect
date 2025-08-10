// SPDX-License-Identifier: GPL-3.0-or-later
#include <cassert>
#include <climits>
#include <iostream>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#ifdef __APPLE__
#ifdef __i386
#define OV_ENDIAN_ARG 0
#else
#define OV_ENDIAN_ARG 1
#endif
#endif
#ifdef __linux__
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define OV_ENDIAN_ARG 0
#else
#define OV_ENDIAN_ARG 1
#endif
#else
#define OV_ENDIAN_ARG 0
#endif

#include <textidentificationframe.h>
#include <vorbisfile.h>
#include <xiphcomment.h>

#include "trackoggvorbis.h"

using namespace std;
using namespace soundtouch;

TrackOggVorbis::TrackOggVorbis(const char *fname, bool readtags) : Track() {
    fptr = nullptr;
    setFilename(fname, readtags);
}

TrackOggVorbis::~TrackOggVorbis() {
    close();
}

void TrackOggVorbis::open() {
    close();

    m_iCurPosPCM = 0;
    string fname = filename();
    // Try to open the file for reading
    fptr = fopen(fname.c_str(), "rb");
    if (fptr == NULL) {
#ifdef DEBUG
        cerr << "TrackOggVorbis: can not open file" << endl;
#endif
        return;
    }

    setOpened(true);
    if (ov_open(fptr, &vf, NULL, 0) < 0) {
#ifdef DEBUG
        cerr << "TrackOggVorbis: Input does not appear to be an Ogg bitstream" << endl;
#endif
        close();
        return;
    }

    // extract metadata
    vorbis_info *vi = ov_info(&vf, -1);

    int channels = vi->channels;
    uint srate = static_cast<uint>(vi->rate);
    unsigned long long numSamples = static_cast<unsigned long long>(ov_pcm_total(&vf, -1));
    uint len = static_cast<uint>(1000 * numSamples / srate);

    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSamplerate(static_cast<int>(srate));
    setSampleBytes(2);
    setChannels(channels);
    setTrackType(TYPE_OGGVORBIS);
    setValid(true);
}

void TrackOggVorbis::close() {
    if (isOpened())
        ov_clear(&vf);
    // note that fclose() is not needed, ov_clear() does this as well
    fptr = NULL;
    m_iCurPosPCM = 0;
    setOpened(false);
}

void TrackOggVorbis::seek(uint ms) {
    if (isValid()) {
        unsigned long long pos =
            (static_cast<uint>(ms) * static_cast<uint>(samplerate()) /* * channels()*/) / 1000;
        if (ov_pcm_seek(&vf, static_cast<ogg_int64_t>(pos)) == 0) {
            m_iCurPosPCM = pos;
        }
#ifdef DEBUG
        else {
            cerr << "seek failed: seek ERR.";
        }
    } else {
        cerr << "seek failed: track not valid" << endl;
#endif
    }
}

uint TrackOggVorbis::currentPos() {
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
 * @return number of read samples
 */
int TrackOggVorbis::readSamples(SAMPLETYPE *buffer, size_t num) {
    if (!isValid() || num < 2)
        return -1;

    short dest[num];
    long index = 0;
    int needed = static_cast<int>(num);
    // loop until requested number of samples has been retrieved
    while (needed > 0) {
        // read samples into buffer
        long ret = ov_read(&vf,
                           reinterpret_cast<char *>(dest) + static_cast<std::ptrdiff_t>(index),
                           needed,
                           OV_ENDIAN_ARG,
                           2,
                           1,
                           &current_section);
        // if eof we fill the rest with zero
        if (ret == 0) {
            while (needed > 0) {
                dest[index] = 0;
                index++;
                needed--;
            }
        }
        index += ret;
        needed -= ret;
    }

    int nread = static_cast<int>(index / 2);
    for (int i = 0; i < nread; ++i) {
        buffer[i] = static_cast<float>(dest[i]) / SAMPLE_MAXVALUE;
    }

    // return the number of samples in buffer
    m_iCurPosPCM += static_cast<unsigned long long>(nread);
    return nread;
}

void TrackOggVorbis::storeBPM(string format) {
    string fname = filename();
    string sBPM = bpm2str(getBPM(), format);
    TagLib::Ogg::Vorbis::File f(fname.c_str(), false);
    TagLib::Ogg::XiphComment *tag = f.tag();
    if (tag == NULL) {
        cerr << "BPM not saved ! (failed)" << endl;
        return;
    }
    tag->addField("TBPM", sBPM.c_str(), true); // add new BPM field (replace existing)
    f.save();
}

void TrackOggVorbis::readTags() {
    string fname = filename();
    string sbpm = "000.00";
    TagLib::Ogg::Vorbis::File f(fname.c_str(), false);
    TagLib::Ogg::XiphComment *tag = f.tag();
    if (tag != NULL) {
        setArtist(tag->artist().toCString());
        setTitle(tag->title().toCString());
        TagLib::Ogg::FieldListMap flmap = tag->fieldListMap();
        TagLib::StringList strl = flmap["TBPM"];
        if (!strl.isEmpty())
            sbpm = strl[0].toCString();
    }
    // set filename (without path) as title if the title is empty
    if (title().empty())
        setTitle(fname.substr(fname.find_last_of("/") + 1));
    setBPM(str2bpm(sbpm));
}

void TrackOggVorbis::removeBPM() {
    string fname = filename();
    //close();
    TagLib::Ogg::Vorbis::File f(fname.c_str(), false);
    TagLib::Ogg::XiphComment *tag = f.tag();
    if (tag == NULL) {
        return;
    }
    tag->removeFields("TBPM");
    f.save();
    //open();
}
