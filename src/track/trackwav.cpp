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

static const char fmtStr[] = "fmt ";
static const char dataStr[] = "data";

using namespace std;
using namespace soundtouch;

static bool isAlphaStr(const std::string &str) {
    for (char c : str) {
        if (!isalpha(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

TrackWav::TrackWav(const QString &fname, bool readtags) : Track() {
    m_iCurPosBytes = 0;
    setFilename(fname, readtags);
}

TrackWav::~TrackWav() {
    close();
}

void TrackWav::open() {
    close();
    auto fname = filename();

    // Try to open the file for reading
    fptr.setFileName(fname);
    if (!fptr.open(QIODevice::ReadOnly)) {
        return;
    }

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
    if (fptr.isOpen()) {
        fptr.close();
    }
    m_iCurPosBytes = 0;
    setOpened(false);
}

void TrackWav::seek(qint64 ms) {
    if (isValid()) {
        fptr.seek(0);
        auto pos = (ms * samplerate() * sampleBytes()) / 1000;
        int hdrsOk = readWavHeaders();
        assert(hdrsOk == 0);
        fptr.seek(pos);
        m_iCurPosBytes = pos;
#ifdef DEBUG
    } else {
        cerr << "seek failed: track not valid" << endl;
#endif
    }
}

qint64 TrackWav::currentPos() {
    if (isValid()) {
        auto pos = 1000 * m_iCurPosBytes / (samplerate() * channels() * sampleBytes());
        return pos;
    }
    return 0;
}

/**
 * Read @a num samples into @a buffer
 * @param buffer pointer to buffer
 * @return number of read samples
 */
int TrackWav::readSamples(std::span<soundtouch::SAMPLETYPE> buffer) {
    if (!isValid())
        return -1;
    return static_cast<int>(read(buffer));
}

qint64 TrackWav::read(std::span<char> buffer) {
    auto maxElems = buffer.size();
    qint64 numBytes;
    qint64 afterDataRead;

    // ensure it's 8 bit format
    if (header.format.bits_per_sample != 8) {
        return -1;
    }

    numBytes = static_cast<qint64>(maxElems);
    afterDataRead = m_iCurPosBytes + numBytes;
    if (afterDataRead > header.data.data_len) {
        // Do not read more samples than are marked available in header
        numBytes = header.data.data_len - m_iCurPosBytes;
        assert(numBytes >= 0);
    }
    auto bytesRead = fptr.read(buffer.data(), numBytes);
    m_iCurPosBytes += bytesRead;

    return bytesRead;
}

qint64 TrackWav::read(std::span<short> buffer) {
    auto maxElems = buffer.size();
    qint64 afterDataRead;
    int numBytes;
    qint64 numElems;

    if (header.format.bits_per_sample == 8) {
        // 8 bit format
        QList<char> temp(static_cast<qsizetype>(maxElems));

        numElems = read(std::span(temp.data(), temp.size()));
        // convert from 8 to 16 bit
        for (size_t i = 0; i < static_cast<size_t>(numElems); i++) {
            buffer[i] = static_cast<short>(temp[i] << 8);
        }
    } else {
        // 16 bit format
        assert(header.format.bits_per_sample == 16);
        assert(sizeof(short) == 2);

        numBytes = static_cast<int>(maxElems * 2);
        afterDataRead = m_iCurPosBytes + numBytes;
        if (afterDataRead > header.data.data_len) {
            // Don't read more samples than are marked available in header
            numBytes = static_cast<int>(header.data.data_len - m_iCurPosBytes);
            assert(numBytes >= 0);
        }

        auto bytesRead = fptr.read(reinterpret_cast<char *>(buffer.data()), numBytes);
        m_iCurPosBytes += bytesRead;
        numElems = static_cast<int>(bytesRead / 2);

        // 16bit samples, swap byte order if necessary
        for (int i = 0; i < numElems; i++)
            buffer[i] = qToLittleEndian(buffer[i]);
    }

    return numElems;
}

qint64 TrackWav::read(std::span<float> buffer) {
    auto maxElems = buffer.size();
    QList<short> temp(static_cast<qsizetype>(maxElems));
    qint64 num;
    int i;
    auto fscale = 1.0 / SAMPLE_MAXVALUE;

    num = read(std::span<short>(temp.data(), temp.size()));
    // convert to floats, scale to range [-1..+1[
    for (i = 0; i < num; i++) {
        buffer[i] = static_cast<float>(fscale * static_cast<double>(temp[i]));
    }

    return num;
}

void TrackWav::storeBPM(const QString &format) {
    auto fname = filename();
    auto sBPM = bpm2str(getBPM(), format);
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
    auto fname = filename();
    auto sbpm = QString::fromUtf8("000.00");
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
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QLatin1Char('/')) + 1));
    setBPM(str2bpm(sbpm));
    open();
}

void TrackWav::removeBPM() {
    auto fname = filename();
    close();
    // TODO
    open();
}

int TrackWav::readRIFFBlock() {
    auto read = fptr.read(reinterpret_cast<char *>(&header.riff), sizeof(WavRiff));
    assert(read > 0);

    // swap 32bit data byte order if necessary
    header.riff.package_len = qToLittleEndian(header.riff.package_len);

    if (header.riff.riff_char[0] == 'R' && header.riff.riff_char[1] == 'I' &&
        header.riff.riff_char[2] == 'F' && header.riff.riff_char[3] == 'F' &&
        header.riff.wave[0] == 'W' && header.riff.wave[1] == 'A' && header.riff.wave[2] == 'V' &&
        header.riff.wave[3] == 'E') {
        return 0;
    }

    return -1;
}

int TrackWav::readHeaderBlock() {
    char label[5];
    string sLabel;

    // lead label string
    auto read = fptr.read(label, 4);
    assert(read > 0);
    label[4] = 0;

    if (!isAlphaStr(std::string(label)))
        return -1; // not a valid label

    // Decode blocks according to their label
    if (strcmp(label, fmtStr) == 0) {
        int nLen, nDump;

        // 'fmt ' block
        memcpy(header.format.fmt, fmtStr, 4);

        // read length of the format field
        auto read_len = fptr.read(reinterpret_cast<char *>(&nLen), sizeof(int));
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
        read_len = fptr.read(reinterpret_cast<char *>(&(header.format.fixed)), nLen);
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
            fptr.skip(nDump);
        }

        return 0;
    } else if (strcmp(label, dataStr) == 0) {
        // 'data' block
        memcpy(header.data.data_field, dataStr, 4);
        read = fptr.read(reinterpret_cast<char *>(&(header.data.data_len)), sizeof(uint));
        assert(read > 0);

        // swap byte order if necessary
        header.data.data_len = qToLittleEndian(header.data.data_len);

        return 1;
    } else {
        uint len, i;
        uint temp;
        // unknown block

        // read length
        read = fptr.read(reinterpret_cast<char *>(&len), sizeof(len));
        assert(read > 0);
        // scan through the block
        for (i = 0; i < len; i++) {
            read = fptr.read(reinterpret_cast<char *>(&temp), 1);
            assert(read > 0);
            if (fptr.atEnd())
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
