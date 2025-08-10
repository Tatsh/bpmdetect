// SPDX-License-Identifier: GPL-3.0-or-later
#include <cassert>
#include <cctype>
#include <climits>
#include <cstring>
#include <iostream>

#include <QtEndian>
#include <id3v2frame.h>
#include <id3v2tag.h>
#include <mpegfile.h>

#include "trackwav.h"

static const char riffStr[] = "RIFF";
static const char waveStr[] = "WAVE";
static const char fmtStr[] = "fmt ";
static const char dataStr[] = "data";

using namespace std;
using namespace soundtouch;

static bool isAlphaStr(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]))
            return false;
    }
    return true;
}

TrackWav::TrackWav(const char *fname, bool readtags) : Track() {
    m_iCurPosBytes = 0;
    fptr = nullptr;
    setFilename(fname, readtags);
}

TrackWav::~TrackWav() {
    close();
}

void TrackWav::open() {
    close();
    string fname = filename();

    // Try to open the file for reading
    fptr = fopen(fname.c_str(), "rb");
    if (fptr == NULL)
        return;

    // Read the file headers
    int hdrsOk = readWavHeaders();
    if (hdrsOk != 0) {
        // Something didn't match in the wav file headers
        close();
    }

    if (header.format.fixed != 1) {
        // Unsupported encoding
        close();
        return;
    }

    m_iCurPosBytes = 0;

    unsigned long long numSamples =
        header.data.data_len / static_cast<unsigned long long>(header.format.byte_per_sample);
    int srate = header.format.sample_rate;
    int channels = header.format.channel_number;

    uint len = (1000 * static_cast<uint>(numSamples) / static_cast<uint>(srate));
    int sbytes = header.format.bits_per_sample;

    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSamplerate(srate);
    setSampleBytes(sbytes);
    setChannels(channels);
    setTrackType(TYPE_WAV);
    setValid(true);
    setOpened(true);
}

void TrackWav::close() {
    if (fptr)
        fclose(fptr);
    fptr = NULL;
    m_iCurPosBytes = 0;
    setOpened(false);
}

void TrackWav::seek(uint ms) {
    if (isValid()) {
        fseek(fptr, 0, SEEK_SET);
        unsigned long long pos =
            (static_cast<unsigned long long>(ms) * static_cast<unsigned long long>(samplerate()) *
             static_cast<unsigned long long>(sampleBytes())) /
            1000ULL;
        int hdrsOk = readWavHeaders();
        assert(hdrsOk == 0);
        fseek(fptr, static_cast<long>(pos), SEEK_CUR);
        m_iCurPosBytes = pos;
#ifdef DEBUG
    } else {
        cerr << "seek failed: track not valid" << endl;
#endif
    }
}

uint TrackWav::currentPos() {
    if (isValid()) {
        unsigned long long pos = 1000ULL * m_iCurPosBytes /
                                 (static_cast<unsigned long long>(samplerate()) *
                                  static_cast<unsigned long long>(channels()) *
                                  static_cast<unsigned long long>(sampleBytes()));
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
int TrackWav::readSamples(std::span<soundtouch::SAMPLETYPE> buffer) {
    if (!isValid())
        return -1;

    int nread = read(buffer.data(), buffer.size());
    return nread;
}

int TrackWav::read(char *buffer, size_t maxElems) {
    unsigned long long numBytes;
    unsigned long long afterDataRead;

    // ensure it's 8 bit format
    if (header.format.bits_per_sample != 8) {
        return -1;
    }
    assert(sizeof(char) == 1);

    numBytes = maxElems;
    afterDataRead = m_iCurPosBytes + numBytes;
    if (afterDataRead > header.data.data_len) {
        // Do not read more samples than are marked available in header
        numBytes = header.data.data_len - m_iCurPosBytes;
        assert(numBytes >= 0);
    }

    size_t bytesRead = fread(buffer, 1, static_cast<size_t>(numBytes), fptr);
    m_iCurPosBytes += static_cast<unsigned long long>(bytesRead);

    return static_cast<int>(bytesRead);
}

int TrackWav::read(short *buffer, size_t maxElems) {
    unsigned long long afterDataRead;
    int numBytes;
    int numElems;

    if (header.format.bits_per_sample == 8) {
        // 8 bit format
        char *temp = new char[maxElems];
        int i;

        numElems = read(temp, maxElems);
        // convert from 8 to 16 bit
        for (i = 0; i < numElems; i++) {
            buffer[i] = static_cast<short>(temp[i] << 8);
        }
        delete[] temp;
    } else {
        // 16 bit format
        assert(header.format.bits_per_sample == 16);
        assert(sizeof(short) == 2);

        numBytes = static_cast<int>(maxElems * 2);
        afterDataRead = m_iCurPosBytes + static_cast<unsigned long long>(numBytes);
        if (afterDataRead > header.data.data_len) {
            // Don't read more samples than are marked available in header
            numBytes = static_cast<int>(header.data.data_len - m_iCurPosBytes);
            assert(numBytes >= 0);
        }

        size_t bytesRead = fread(buffer, 1, static_cast<size_t>(numBytes), fptr);
        m_iCurPosBytes += static_cast<unsigned long long>(bytesRead);
        numElems = static_cast<int>(bytesRead / 2);

        // 16bit samples, swap byte order if necessary
        for (int i = 0; i < numElems; i++)
            buffer[i] = qToLittleEndian(buffer[i]);
    }

    return numElems;
}

int TrackWav::read(float *buffer, size_t maxElems) {
    short *temp = new short[maxElems];
    int num;
    int i;
    double fscale;

    num = read(temp, maxElems);

    fscale = 1.0 / SAMPLE_MAXVALUE;
    // convert to floats, scale to range [-1..+1[
    for (i = 0; i < num; i++) {
        buffer[i] = static_cast<float>(fscale * static_cast<double>(temp[i]));
    }

    delete[] temp;
    return num;
}

void TrackWav::storeBPM(string format) {
    string fname = filename();
    string sBPM = bpm2str(getBPM(), format);
    close();
    /*
    TagLib::MPEG::File f( fname.c_str(), false );
    long offset = f.rfind("ID3", TagLib::File::End);
    if(offset < 0) offset = f.length();           // ID3 tag offset
    TagLib::ID3v2::Tag tag(&f, offset);
    tag.removeFrames("TBPM");                     // remove existing BPM frames

    TagLib::ID3v2::TextIdentificationFrame* bpmframe =
        new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
    bpmframe->setText(sBPM.c_str());
    tag.addFrame(bpmframe);                       // add new BPM frame

    TagLib::ByteVector tdata = tag.render();      // render tag to binary data
    f.seek(offset);
    f.writeBlock(tdata);                          // write to file
    //f.save();
    */
    open();
}

void TrackWav::readTags() {
    string fname = filename();
    string sbpm = "000.00";
    /*
      TagLib::MPEG::File f(fname.c_str(), false);
      long pos = f.rfind("ID3", TagLib::File::End);
      if(pos < 0) pos = f.length();

      TagLib::ID3v2::Tag tag(&f, pos);
      setArtist(tag.artist().toCString());
      setTitle(tag.title().toCString());

      TagLib::List<TagLib::ID3v2::Frame*> lst = tag.frameList("TBPM");
      if(lst.size() > 0) {
        TagLib::ID3v2::Frame* frame = lst[0];
        sbpm = frame->toString().toCString();
      }
    */
    // set filename (without path) as title if the title is empty
    if (title().empty())
        setTitle(fname.substr(fname.find_last_of("/") + 1));
    setBPM(str2bpm(sbpm));
    open();
}

void TrackWav::removeBPM() {
    string fname = filename();
    close();
    // TODO
    open();
}

int TrackWav::readRIFFBlock() {
    size_t read = fread(&(header.riff), sizeof(WavRiff), 1, fptr);
    assert(read > 0);

    // swap 32bit data byte order if necessary
    header.riff.package_len = qToLittleEndian(header.riff.package_len);

    // header.riff.riff_char should equal to 'RIFF');
    if (memcmp(riffStr, header.riff.riff_char, 4) != 0)
        return -1;
    // header.riff.wave should equal to 'WAVE'
    if (memcmp(waveStr, header.riff.wave, 4) != 0)
        return -1;

    return 0;
}

int TrackWav::readHeaderBlock() {
    char label[5];
    string sLabel;

    // lead label string
    size_t read = fread(label, 1, 4, fptr);
    assert(read > 0);
    label[4] = 0;

    if (!isAlphaStr(label))
        return -1; // not a valid label

    // Decode blocks according to their label
    if (strcmp(label, fmtStr) == 0) {
        int nLen, nDump;

        // 'fmt ' block
        memcpy(header.format.fmt, fmtStr, 4);

        // read length of the format field
        size_t read_len = fread(&nLen, sizeof(int), 1, fptr);
        assert(read_len > 0);
        // swap byte order if necessary
        nLen = qToLittleEndian(nLen); // int format_len;
        header.format.format_len = nLen;

        // calculate how much length differs from expected
        nDump = static_cast<int>(nLen - static_cast<int>(sizeof(header.format) - 8));

        // if format_len is larger than expected, read only as much data as we've space for
        if (nDump > 0) {
            nLen = sizeof(header.format) - 8;
        }

        // read data
        read_len = fread(&(header.format.fixed), static_cast<size_t>(nLen), 1, fptr);
        assert(read_len > 0);

        // swap byte order if necessary
        header.format.fixed = qToLittleEndian(header.format.fixed); // short int fixed;
        header.format.channel_number =
            qToLittleEndian(header.format.channel_number);                      // short int channel
        header.format.sample_rate = qToLittleEndian(header.format.sample_rate); // int sample_rate;
        header.format.byte_rate = qToLittleEndian(header.format.byte_rate);     // int byte_rate
        header.format.byte_per_sample = qToLittleEndian(header.format.byte_per_sample); // short
        header.format.bits_per_sample =
            qToLittleEndian(header.format.bits_per_sample); // short bits_per_sample

        // if format_len is larger than expected, skip the extra data
        if (nDump > 0) {
            fseek(fptr, nDump, SEEK_CUR);
        }

        return 0;
    } else if (strcmp(label, dataStr) == 0) {
        // 'data' block
        memcpy(header.data.data_field, dataStr, 4);
        read = fread(&(header.data.data_len), sizeof(uint), 1, fptr);
        assert(read > 0);

        // swap byte order if necessary
        header.data.data_len = qToLittleEndian(header.data.data_len);

        return 1;
    } else {
        uint len, i;
        uint temp;
        // unknown block

        // read length
        read = fread(&len, sizeof(len), 1, fptr);
        assert(read > 0);
        // scan through the block
        for (i = 0; i < len; i++) {
            read = fread(&temp, 1, 1, fptr);
            assert(read > 0);
            if (feof(fptr))
                return -1; // unexpected eof
        }
    }
    return 0;
}

int TrackWav::readWavHeaders() {
    int res;

    memset(&header, 0, sizeof(header));

    res = readRIFFBlock();
    if (res)
        return 1;
    // read header blocks until data block is found
    do {
        // read header blocks
        res = readHeaderBlock();
        if (res < 0)
            return 1; // error in file structure
    } while (res == 0);
    // check that all required tags are legal
    return checkCharTags();
}

int TrackWav::checkCharTags() {
    // header.format.fmt should equal to 'fmt '
    if (memcmp(fmtStr, header.format.fmt, 4) != 0)
        return -1;
    // header.data.data_field should equal to 'data'
    if (memcmp(dataStr, header.data.data_field, 4) != 0)
        return -1;

    return 0;
}
