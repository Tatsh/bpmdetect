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

#pragma once

#include "track.h"

/// WAV audio file 'riff' section header
typedef struct {
    char riff_char[4];
    int package_len;
    char wave[4];
} WavRiff;

/// WAV audio file 'format' section header
typedef struct {
    char fmt[4];
    int format_len;
    short fixed;
    short channel_number;
    int sample_rate;
    int byte_rate;
    short byte_per_sample;
    short bits_per_sample;
} WavFormat;

/// WAV audio file 'data' section header
typedef struct {
    char data_field[4];
    uint data_len;
} WavData;

/// WAV audio file header
typedef struct {
    WavRiff riff;
    WavFormat format;
    WavData data;
} WavHeader;

class TrackWav : public Track {
public:
    TrackWav(const char *filename, bool readtags = true);
    ~TrackWav();
    void readTags();

protected:
    void open();
    void close();
    void seek(uint ms);
    uint currentPos();
    int readSamples(soundtouch::SAMPLETYPE *buffer, unsigned int num);

    void storeBPM(std::string sBPM);
    void removeBPM();

    int readWavHeaders();
    int readHeaderBlock();
    int readRIFFBlock();
    int checkCharTags();
    int read(char *buffer, int maxElems);
    int read(short *buffer, int maxElems);
    int read(float *buffer, int maxElems);

private:
    unsigned long long m_iCurPosBytes;
    FILE *fptr;
    WavHeader header;
};
