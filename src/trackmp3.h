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

#ifndef TRACKMP3_H
#define TRACKMP3_H

#include "track.h"
#include <mad.h>

#include <vector>

/** Struct used to store mad frames for seeking */
typedef struct MadSeekFrameType {
    unsigned char *m_pStreamPos;
    long int pos;
} MadSeekFrameType;


class TrackMp3 : public Track {
public:
    TrackMp3( const char* filename, bool readtags = true );
    ~TrackMp3();
    void readTags();

protected:
    void open();
    void close();
    void seek( uint ms );
    uint currentPos();
    int readSamples( soundtouch::SAMPLETYPE* buffer, int num );
    inline signed int madScale(mad_fixed_t sample);

    void storeBPM( std::string sBPM );
    void removeBPM();

    void clearFrameList();
    inline unsigned long madLength();

private:
    unsigned long discard(unsigned long samples_wanted);
    int findFrame(int pos);

    unsigned long long m_iCurPosPCM;
    FILE *fptr;
    unsigned int inputbuf_len;
    unsigned char *inputbuf;
    int framecount;
    int currentframe;
    int bitrate;
    mad_timer_t pos;
    mad_timer_t filelength;
    mad_stream stream;
    mad_frame *frame;
    mad_synth synth;
    // Start index in Synth buffer of samples left over from previous call to readSamples
    int rest;
    // Average frame size used when searching for a frame
    int m_iAvgFrameSize;

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
     * To have precise seek within a limited range from the current decode position, we keep track
     * of past decodeded frame, and their exact position. If a seek occours and it is within the
     * range of frames we keep track of a precise seek occours, otherwise an unprecise seek is performed
     */
    std::vector<MadSeekFrameType*> m_qSeekList;
};

#endif  // TRACKMP3_H
