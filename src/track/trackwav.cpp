// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QtEndian>
#include <id3v2frame.h>
#include <id3v2tag.h>
#include <mpegfile.h>

#include "trackwav.h"

const QString fmtStr = QStringLiteral("fmt ");
const QString dataStr = QStringLiteral("data");

static inline auto isAlphaStr(const QString &str) {
    for (char c : str.toUtf8()) {
        if (!isalpha(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

TrackWav::TrackWav(const QString &fname, bool readMetadata) : Track() {
    m_iCurPosBytes = 0;
    setFileName(fname, readMetadata);
}

TrackWav::~TrackWav() {
    close();
}

void TrackWav::open() {
    close();
    auto fname = fileName();

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

    auto numSamples =
        header.data.data_len / static_cast<unsigned short>(header.format.byte_per_sample);
    auto sRate = static_cast<unsigned int>(header.format.sample_rate);
    auto channels = static_cast<unsigned int>(header.format.channel_number);

    auto len = 1000 * numSamples / sRate;
    auto sbytes = header.format.bits_per_sample;

    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSampleRate(sRate);
    setSampleBytes(static_cast<unsigned int>(sbytes));
    setChannels(channels);
    setTrackType(Wave);
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

void TrackWav::seek(quint64 ms) {
    if (isValid()) {
        fptr.seek(0);
        auto pos = (ms * sampleRate() * sampleBytes()) / 1000;
        auto hdrsOk = readWavHeaders();
        Q_ASSERT(hdrsOk == 0);
        fptr.seek(static_cast<qint64>(pos));
        m_iCurPosBytes = pos;
    } else {
        qCritical() << "seek failed: track not valid";
    }
}

quint64 TrackWav::currentPos() {
    if (isValid()) {
        auto pos = 1000 * m_iCurPosBytes / (sampleRate() * channels() * sampleBytes());
        return pos;
    }
    return 0;
}

/**
 * Read @a num samples into @a buffer
 * @param buffer pointer to buffer
 * @return number of read samples
 */
int TrackWav::readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) {
    if (!isValid())
        return -1;
    return static_cast<int>(read(buffer));
}

qint64 TrackWav::read(QSpan<char> buffer) {
    auto maxElems = buffer.size();
    quint64 numBytes;
    quint64 afterDataRead;

    // ensure it's 8 bit format
    if (header.format.bits_per_sample != 8) {
        return -1;
    }

    numBytes = static_cast<quint64>(maxElems);
    afterDataRead = m_iCurPosBytes + numBytes;
    if (afterDataRead > header.data.data_len) {
        // Do not read more samples than are marked available in header
        numBytes = header.data.data_len - m_iCurPosBytes;
        Q_ASSERT(numBytes >= 0);
    }
    auto bytesRead = fptr.read(buffer.data(), static_cast<qint64>(numBytes));
    m_iCurPosBytes += static_cast<quint64>(bytesRead);

    return bytesRead;
}

qint64 TrackWav::read(QSpan<short> buffer) {
    auto maxElems = buffer.size();
    quint64 afterDataRead;
    int numBytes;
    qint64 numElems;

    if (header.format.bits_per_sample == 8) {
        // 8 bit format
        QList<char> temp(maxElems);

        numElems = read(QSpan(temp.data(), temp.size()));
        // convert from 8 to 16 bit
        for (qsizetype i = 0; i < static_cast<qsizetype>(numElems); i++) {
            buffer[i] = static_cast<short>(temp[i] << 8);
        }
    } else {
        // 16 bit format
        Q_ASSERT(header.format.bits_per_sample == 16);
        Q_ASSERT(sizeof(short) == 2);

        numBytes = static_cast<int>(maxElems * 2);
        afterDataRead = m_iCurPosBytes + static_cast<quint64>(numBytes);
        if (afterDataRead > header.data.data_len) {
            // Don't read more samples than are marked available in header
            numBytes = static_cast<int>(header.data.data_len - m_iCurPosBytes);
            Q_ASSERT(numBytes >= 0);
        }

        auto bytesRead = fptr.read(reinterpret_cast<char *>(buffer.data()), numBytes);
        m_iCurPosBytes += static_cast<quint64>(bytesRead);
        numElems = bytesRead / 2;

        // 16bit samples, swap byte order if necessary
        for (qsizetype i = 0; i < numElems; i++)
            buffer[i] = qToLittleEndian(buffer[i]);
    }

    return numElems;
}

qint64 TrackWav::read(QSpan<float> buffer) {
    auto maxElems = buffer.size();
    QList<short> temp(maxElems);
    qint64 num;
    qsizetype i;
    auto fscale = 1.0 / SAMPLE_MAX_VALUE;

    num = read(QSpan<short>(temp.data(), temp.size()));
    // convert to floats, scale to range [-1..+1[
    for (i = 0; i < static_cast<qsizetype>(num); i++) {
        buffer[i] = static_cast<float>(fscale * static_cast<double>(temp[i]));
    }

    return num;
}

void TrackWav::storeBpm(const QString &format) {
    auto fname = fileName();
    auto sBpm = bpmToString(bpm(), format);
    close();
    /*
    TagLib::MPEG::File f( fname.c_str(), false );
    long offset = f.rfind("ID3", TagLib::File::End);
    if(offset < 0) offset = f.length();           // ID3 tag offset
    TagLib::ID3v2::Tag tag(&f, offset);
    tag.removeFrames("TBPM");                     // remove existing BPM frames

    TagLib::ID3v2::TextIdentificationFrame* bpmFrame =
        new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
    bpmFrame->setText(sBpm.c_str());
    tag.addFrame(bpmFrame);                       // add new BPM frame

    TagLib::ByteVector tdata = tag.render();      // render tag to binary data
    f.seek(offset);
    f.writeBlock(tdata);                          // write to file
    //f.save();
    */
    open();
}

void TrackWav::readTags() {
    auto fname = fileName();
    auto sBpm = QStringLiteral("000.00");
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
        sBpm = frame->toString().toCString();
      }
    */
    // set fileName (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBpm(stringToBpm(sBpm));
    open();
}

void TrackWav::removeBpm() {
    auto fname = fileName();
    close();
    // TODO
    open();
}

int TrackWav::readRIFFBlock() {
    auto read = fptr.read(reinterpret_cast<char *>(&header.riff), sizeof(WavRiff));
    Q_ASSERT(read > 0);

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

    // lead label string
    auto read = fptr.read(label, 4);
    auto sLabel = QString::fromUtf8(label);
    Q_ASSERT(read > 0);
    label[4] = 0;

    if (!isAlphaStr(sLabel))
        return -1; // not a valid label

    // Decode blocks according to their label
    if (sLabel == fmtStr) {
        int nLen, nDump;

        // 'fmt ' block
        std::copy(fmtStr.toLocal8Bit().begin(), fmtStr.toLocal8Bit().end(), header.format.fmt);

        // read length of the format field
        auto read_len = fptr.read(reinterpret_cast<char *>(&nLen), sizeof(int));
        Q_ASSERT(read_len > 0);
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
        Q_ASSERT(read_len > 0);

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
    } else if (sLabel == dataStr) {
        // 'data' block
        std::copy(
            dataStr.toLocal8Bit().begin(), dataStr.toLocal8Bit().end(), header.data.data_field);
        read = fptr.read(reinterpret_cast<char *>(&(header.data.data_len)), sizeof(unsigned int));
        Q_ASSERT(read > 0);

        // swap byte order if necessary
        header.data.data_len = qToLittleEndian(header.data.data_len);

        return 1;
    } else {
        unsigned int len, i, temp;
        // unknown block

        // read length
        read = fptr.read(reinterpret_cast<char *>(&len), sizeof(len));
        Q_ASSERT(read > 0);
        // scan through the block
        for (i = 0; i < len; i++) {
            read = fptr.read(reinterpret_cast<char *>(&temp), 1);
            Q_ASSERT(read > 0);
            if (fptr.atEnd())
                return -1; // unexpected eof
        }
    }
    return 0;
}

int TrackWav::readWavHeaders() {
    int res;

    header = {};

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
    if (fmtStr != QString::fromLocal8Bit(header.format.fmt))
        return -1;
    // header.data.data_field should equal to 'data'
    if (fmtStr != QString::fromLocal8Bit(header.data.data_field))
        return -1;

    return 0;
}
