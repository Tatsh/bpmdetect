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
    fptr = 0;
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
    uint srate = vi->rate;
    unsigned long long numSamples = ov_pcm_total(&vf, -1);
    uint len = (1000 * numSamples / srate);

    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSamplerate(srate);
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
        unsigned long long pos = (ms * samplerate() /* * channels()*/) / 1000;
        if (ov_pcm_seek(&vf, pos) == 0) {
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
        unsigned long long pos = 1000 * m_iCurPosPCM / (samplerate() /* *channels()*/);
        return (uint)pos;
    }
    return 0;
}

/**
 * Read @a num samples into @a buffer
 * @param buffer pointer to buffer
 * @param num number of samples (per channel)
 * @return number of read samples
 */
int TrackOggVorbis::readSamples(SAMPLETYPE *buffer, unsigned int num) {
    if (!isValid() || num < 2)
        return -1;

    short dest[num];
    uint index = 0;
    int needed = num;
    // loop until requested number of samples has been retrieved
    while (needed > 0) {
        // read samples into buffer
        int ret = ov_read(&vf, (char *)dest + index, needed, OV_ENDIAN_ARG, 2, 1, &current_section);
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

    int nread = index / 2;
    for (int i = 0; i < nread; ++i) {
        buffer[i] = (float)dest[i] / 32768;
    }

    // return the number of samples in buffer
    m_iCurPosPCM += nread;
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
