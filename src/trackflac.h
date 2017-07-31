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

#ifndef TRACKFLAC_H
#define TRACKFLAC_H

#include "track.h"

#include "FLAC/stream_decoder.h"

typedef struct {
    FLAC__uint64 total_samples;
    int channels;           // number of channels
    unsigned int srate;     // sample rate
    unsigned bps;           // bits per sample

    short* buffer;          // buffer of samples (16 bit)
    uint bufsize;           // buffer size (maximum number of samples)
    uint numsamples;        // number of samples in buffer
} FLAC_CLIENT_DATA;

class TrackFlac : public Track {
public:
    TrackFlac( const char* filename, bool readtags = true );
    ~TrackFlac();
    void readTags();

protected:
    void open();
    void close();
    void seek( uint ms );
    uint currentPos();
    int readSamples( soundtouch::SAMPLETYPE* buffer, int num );

    void storeBPM( std::string sBPM );
    void removeBPM();

private:
    static FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *decoder,
            const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
    static void metadataCallback(const FLAC__StreamDecoder *decoder,
                                 const FLAC__StreamMetadata *metadata, void *client_data);
    static void errorCallback(const FLAC__StreamDecoder *decoder,
                              FLAC__StreamDecoderErrorStatus status, void *client_data);

    FLAC__StreamDecoder *m_decoder;
    FLAC_CLIENT_DATA m_cldata;

    unsigned long m_ibufidx;
    unsigned long long m_iCurPosPCM;
};

#endif  // TRACKFLAC_H
