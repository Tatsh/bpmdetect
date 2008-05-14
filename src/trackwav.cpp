/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
     e-mail             : martin.sakmar@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "trackwav.h"

#include <assert.h>
#include <limits.h>

#ifdef HAVE_TAGLIB
  #include <mpegfile.h>
  #include <id3v2tag.h>
  #include <id3v2frame.h>
#endif   // HAVE_TAGLIB

static const char riffStr[] = "RIFF";
static const char waveStr[] = "WAVE";
static const char fmtStr[]  = "fmt ";
static const char dataStr[] = "data";

using namespace std;
using namespace soundtouch;

//////////////////////////////////////////////////////////////////////////////
//
// Helper functions for swapping byte order to correctly read/write WAV files 
// with big-endian CPU's: Define compile-time definition _BIG_ENDIAN_ to
// turn-on the conversion if it appears necessary. 
//
// For example, Intel x86 is little-endian and doesn't require conversion,
// while PowerPC of Mac's and many other RISC cpu's are big-endian.

#ifdef BYTE_ORDER
    // In gcc compiler detect the byte order automatically
    #if BYTE_ORDER == BIG_ENDIAN
        // big-endian platform.
        #define _BIG_ENDIAN_
    #endif
#endif
    
#ifdef _BIG_ENDIAN_
    // big-endian CPU, swap bytes in 16 & 32 bit words

    // helper-function to swap byte-order of 32bit integer
    static inline void _swap32(unsigned int &dwData)
    {
        dwData = ((dwData >> 24) & 0x000000FF) | 
                 ((dwData >> 8)  & 0x0000FF00) | 
                 ((dwData << 8)  & 0x00FF0000) | 
                 ((dwData << 24) & 0xFF000000);
    }   

    // helper-function to swap byte-order of 16bit integer
    static inline void _swap16(unsigned short &wData)
    {
        wData = ((wData >> 8) & 0x00FF) | 
                ((wData << 8) & 0xFF00);
    }

    // helper-function to swap byte-order of buffer of 16bit integers
    static inline void _swap16Buffer(unsigned short *pData, unsigned int dwNumWords)
    {
        unsigned long i;

        for (i = 0; i < dwNumWords; i ++)
        {
            _swap16(pData[i]);
        }
    }

#else   // BIG_ENDIAN
    // little-endian CPU, WAV file is ok as such

    // dummy helper-function
    static inline void _swap32(unsigned int &dwData)
    {
        // do nothing
    }   

    // dummy helper-function
    static inline void _swap16(unsigned short &wData)
    {
        // do nothing
    }

    // dummy helper-function
    static inline void _swap16Buffer(unsigned short *pData, unsigned int dwNumBytes)
    {
        // do nothing
    }
#endif  // BIG_ENDIAN

static int isAlpha(char c) {
    return (c >= ' ' && c <= 'z') ? 1 : 0;
}

static int isAlphaStr(char *str) {
    int c = str[0];
    while (c) {
        if (isAlpha(c) == 0) return 0;
        str ++;
        c = str[0];
    }

    return 1;
}

TrackWav::TrackWav( const char* fname, bool readtags ) : Track() {
  m_iCurPosBytes = 0;
  fptr = 0;
  setFilename( fname, readtags );
}

TrackWav::TrackWav( string fname, bool readtags ) : Track() {
  m_iCurPosBytes = 0;
  fptr = 0;
  setFilename( fname, readtags );
}

TrackWav::~TrackWav() {
  close();
}

void TrackWav::open() {
  if(isValid()) close();
  string fname = filename();

  // Try to open the file for reading
  fptr = fopen(fname.c_str(), "rb");
  if (fptr == NULL) return;

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

//    m_iCurPosBytes = 0;
  setValid(true);

  uint numSamples = header.data.data_len / header.format.byte_per_sample;
  uint srate = header.format.sample_rate;
  int channels = header.format.channel_number;

  assert(numSamples < UINT_MAX / 1000);
  uint len =  (1000 * numSamples / srate);
  int sbytes = header.format.bits_per_sample;

  setLength( len );
  setStartPos( 0 );
  setEndPos( len );
  setSamplerate(srate);
  setSampleBytes(sbytes);
  setChannels(channels);
  setTrackType(TYPE_WAV);
  setValid(true);
}

void TrackWav::close() {
  fclose(fptr);
  fptr = NULL;
  m_iCurPosBytes = 0;
  init();
}

void TrackWav::seek( uint ms ) {
  if(isValid()) {
    fseek(fptr, 0, SEEK_SET);
    unsigned long long pos = (ms * samplerate() * sampleBytes()) / 1000;
    int hdrsOk = readWavHeaders();
    assert(hdrsOk == 0);
    fseek(fptr, (uint) pos, SEEK_CUR);
    m_iCurPosBytes = (uint) pos;
#ifdef DEBUG
  } else {
    cerr << "seek failed: track not valid" << endl;
#endif
  }
}

uint TrackWav::currentPos() {
  if(isValid()) {
    unsigned long long pos = 1000*m_iCurPosBytes / (samplerate()*channels()*sampleBytes());
    return (uint) pos;
  }
  return 0;
}

/**
 * Read @a num samples into @a buffer
 * @param buffer pointer to buffer
 * @param num number of samples (per channel)
 * @return number of read samples
 */
int TrackWav::readSamples( SAMPLETYPE* buffer, int num ) {
  if(!isValid()) return -1;

  int nread = read(buffer, num);
  return nread;
}

int TrackWav::read(char *buffer, int maxElems) {
    int numBytes;
    uint afterDataRead;

    // ensure it's 8 bit format
    if (header.format.bits_per_sample != 8) {
        return -1;
    }
    assert(sizeof(char) == 1);

    numBytes = maxElems;
    afterDataRead = m_iCurPosBytes + numBytes;
    if (afterDataRead > header.data.data_len) {
        // Don't read more samples than are marked available in header
        numBytes = header.data.data_len - m_iCurPosBytes;
        assert(numBytes >= 0);
    }

    numBytes = fread(buffer, 1, numBytes, fptr);
    m_iCurPosBytes += numBytes;

    return numBytes;
}

int TrackWav::read(short *buffer, int maxElems) {
    unsigned int afterDataRead;
    int numBytes;
    int numElems;

    if (header.format.bits_per_sample == 8) {
        // 8 bit format
        char *temp = new char[maxElems];
        int i;

        numElems = read(temp, maxElems);
        // convert from 8 to 16 bit
        for (i = 0; i < numElems; i ++) {
            buffer[i] = temp[i] << 8;
        }
        delete[] temp;
    } else {
        // 16 bit format
        assert(header.format.bits_per_sample == 16);
        assert(sizeof(short) == 2);

        numBytes = maxElems * 2;
        afterDataRead = m_iCurPosBytes + numBytes;
        if (afterDataRead > header.data.data_len) {
            // Don't read more samples than are marked available in header
            numBytes = header.data.data_len - m_iCurPosBytes;
            assert(numBytes >= 0);
        }

        numBytes = fread(buffer, 1, numBytes, fptr);
        m_iCurPosBytes += numBytes;
        numElems = numBytes / 2;

        // 16bit samples, swap byte order if necessary
        _swap16Buffer((unsigned short *)buffer, numElems);
    }

    return numElems;
}



int TrackWav::read(float *buffer, int maxElems) {
    short *temp = new short[maxElems];
    int num;
    int i;
    double fscale;

    num = read(temp, maxElems);

    fscale = 1.0 / 32768.0;
    // convert to floats, scale to range [-1..+1[
    for (i = 0; i < num; i ++)
    {
        buffer[i] = (float)(fscale * (double)temp[i]);
    }

    delete[] temp;
    return num;
}

void TrackWav::storeBPM( string format ) {
  string fname = filename();
  string sBPM = bpm2str( getBPM(), format );
#ifdef HAVE_TAGLIB
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
#endif
}

void TrackWav::readTags() {
  string fname = filename();
  string sbpm = "000.00";
#ifdef HAVE_TAGLIB
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
#endif
  // set filename (without path) as title if the title is empty
  if(title().empty())
    setTitle(fname.substr(fname.find_last_of("/") + 1));
  setBPM(str2bpm(sbpm));
  open();
}

void TrackWav::removeBPM() {
  string fname = filename();
#ifdef HAVE_TAGLIB
  close();
  // TODO
  open();
#endif
}


int TrackWav::readRIFFBlock() {
    fread(&(header.riff), sizeof(WavRiff), 1, fptr);

    // swap 32bit data byte order if necessary
    _swap32((unsigned int &)header.riff.package_len);

    // header.riff.riff_char should equal to 'RIFF');
    if (memcmp(riffStr, header.riff.riff_char, 4) != 0) return -1;
    // header.riff.wave should equal to 'WAVE'
    if (memcmp(waveStr, header.riff.wave, 4) != 0) return -1;

    return 0;
}

int TrackWav::readHeaderBlock() {
    char label[5];
    string sLabel;

    // lead label string
    fread(label, 1, 4, fptr);
    label[4] = 0;

    if (isAlphaStr(label) == 0) return -1;    // not a valid label

    // Decode blocks according to their label
    if (strcmp(label, fmtStr) == 0)
    {
        int nLen, nDump;

        // 'fmt ' block 
        memcpy(header.format.fmt, fmtStr, 4);

        // read length of the format field
        fread(&nLen, sizeof(int), 1, fptr);
        // swap byte order if necessary
        _swap32((unsigned int &)nLen); // int format_len;
        header.format.format_len = nLen;

        // calculate how much length differs from expected
        nDump = nLen - (sizeof(header.format) - 8);

        // if format_len is larger than expected, read only as much data as we've space for
        if (nDump > 0)
        {
            nLen = sizeof(header.format) - 8;
        }

        // read data
        fread(&(header.format.fixed), nLen, 1, fptr);

        // swap byte order if necessary
        _swap16((unsigned short &)header.format.fixed);            // short int fixed;
        _swap16((unsigned short &)header.format.channel_number);   // short int channel_number;
        _swap32((unsigned int   &)header.format.sample_rate);      // int sample_rate;
        _swap32((unsigned int   &)header.format.byte_rate);        // int byte_rate;
        _swap16((unsigned short &)header.format.byte_per_sample);  // short int byte_per_sample;
        _swap16((unsigned short &)header.format.bits_per_sample);  // short int bits_per_sample;

        // if format_len is larger than expected, skip the extra data
        if (nDump > 0)
        {
            fseek(fptr, nDump, SEEK_CUR);
        }

        return 0;
    }
    else if (strcmp(label, dataStr) == 0)
    {
        // 'data' block
        memcpy(header.data.data_field, dataStr, 4);
        fread(&(header.data.data_len), sizeof(uint), 1, fptr);

        // swap byte order if necessary
        _swap32((unsigned int &)header.data.data_len);

        return 1;
    }
    else
    {
        uint len, i;
        uint temp;
        // unknown block

        // read length
        fread(&len, sizeof(len), 1, fptr);
        // scan through the block
        for (i = 0; i < len; i ++)
        {
            fread(&temp, 1, 1, fptr);
            if (feof(fptr)) return -1;   // unexpected eof
        }
    }
    return 0;
}


int TrackWav::readWavHeaders() {
    int res;

    memset(&header, 0, sizeof(header));

    res = readRIFFBlock();
    if (res) return 1;
    // read header blocks until data block is found
    do
    {
        // read header blocks
        res = readHeaderBlock();
        if (res < 0) return 1;  // error in file structure
    } while (res == 0);
    // check that all required tags are legal
    return checkCharTags();
}

int TrackWav::checkCharTags() {
    // header.format.fmt should equal to 'fmt '
    if (memcmp(fmtStr, header.format.fmt, 4) != 0) return -1;
    // header.data.data_field should equal to 'data'
    if (memcmp(dataStr, header.data.data_field, 4) != 0) return -1;

    return 0;
}

