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

#include "trackmp3.h"

#ifdef HAVE_TAGLIB
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <textidentificationframe.h>
#endif   // HAVE_TAGLIB

#ifdef HAVE_ID3LIB
#ifdef _WIN32
# define ID3LIB_LINKOPTION 1
#endif
#include <id3/tag.h>
#include <id3/id3lib_streams.h>
#include <id3/readers.h>
#include <id3/misc_support.h>
#endif   // HAVE_ID3LIB

#ifdef __WIN__
#include <io.h>
#include <fcntl.h>
#endif

#ifndef math_min
#define math_min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#include <assert.h>

#include <iostream>

using namespace std;
using namespace soundtouch;

TrackMp3::TrackMp3 ( const char* fname, bool readtags ) : Track() {
  fptr = 0;
  setFilename ( fname, readtags );
}

TrackMp3::~TrackMp3() {
  close();
}

void TrackMp3::clearFrameList() {
  for (int i = 0; i < m_qSeekList.size(); i++ ) {
    MadSeekFrameType* p = m_qSeekList.at(i);
    delete p; p = 0;
  }
  m_qSeekList.clear();
}

void TrackMp3::open() {
  if (isValid() ) close();
  m_iCurPosPCM = 0;
  rest = 0;
  string fname = filename();
  // Try to open the file for reading
  fptr = fopen (fname.c_str(), "rb");
  if (fptr == NULL) {
#ifdef DEBUG
    cerr << "TrackMp3: can not open file" << endl;
#endif
    return;
  }

  // Read the whole file into inputbuf:
  fseek (fptr, 0, SEEK_END);
  inputbuf_len = ftell (fptr);
  fseek (fptr, 0, SEEK_SET);
  inputbuf = new unsigned char[inputbuf_len];
  unsigned int tmp = fread (inputbuf, inputbuf_len, 1, fptr);
#ifdef DEBUG
  if (tmp != 1)
    cerr << "MAD: Error reading mp3-file: " << fname
    << " read only " << tmp << " bytes, but wanted " << inputbuf_len << " bytes" << endl;
#endif
  // Transfer it to the mad stream-buffer:
  mad_stream_init (&stream);
  mad_stream_options (&stream, MAD_OPTION_IGNORECRC);
  mad_stream_buffer (&stream, inputbuf, inputbuf_len);

  // Decode all the headers, and fill in stats:
  mad_header header;
  currentframe = 0;
  filelength = mad_timer_zero;
  pos = mad_timer_zero;

  int channels = 0;
  uint srate = 44100;
  clearFrameList();
  while ( (stream.bufend - stream.this_frame) > 0) {
    if (mad_header_decode (&header, &stream) == -1) {
      if (!MAD_RECOVERABLE (stream.error) ) break;
      continue;
    }

    // Add frame to list of frames
    MadSeekFrameType* p = new MadSeekFrameType;
    p->m_pStreamPos = (unsigned char*) stream.this_frame;
    //p->pos = length();
    m_qSeekList.push_back (p);

    currentframe++;
    mad_timer_add (&filelength, header.duration);
    bitrate += header.bitrate;
    srate = header.samplerate;
    channels = MAD_NCHANNELS (&header);
  }


  // Find average frame size
  if(currentframe) m_iAvgFrameSize = length() / currentframe;
  else m_iAvgFrameSize = 0;

  mad_header_finish (&header);
  if (currentframe == 0)
    bitrate = 0;
  else
    bitrate = bitrate / currentframe;
  framecount = currentframe;
  currentframe = 0;

  frame = new mad_frame;

  // Re-init buffer:
  setValid (true);
  seek (0);

  setSamplerate (srate);
  unsigned long long numSamples = madLength() / channels;
  uint len = (1000 * numSamples / srate);

  setLength ( len );
  setStartPos ( 0 );
  setEndPos ( len );
  setSampleBytes (2);
  setChannels (channels);
  setTrackType (TYPE_MPEG);
}

void TrackMp3::close() {
  if (!isValid() ) return;
  if (fptr) fclose (fptr);
  fptr = NULL;
  m_iCurPosPCM = 0;
  clearFrameList();
  init();
}

inline unsigned long TrackMp3::madLength() {
  enum mad_units units;

  uint srate = samplerate();
  cerr << "TrackMp3 madLength, samplerate = " << srate << endl;
  switch (srate) {
    case 8000:
      units = MAD_UNITS_8000_HZ;
      break;
    case 11025:
      units = MAD_UNITS_11025_HZ;
      break;
    case 12000:
      units = MAD_UNITS_12000_HZ;
      break;
    case 16000:
      units = MAD_UNITS_16000_HZ;
      break;
    case 22050:
      units = MAD_UNITS_22050_HZ;
      break;
    case 24000:
      units = MAD_UNITS_24000_HZ;
      break;
    case 32000:
      units = MAD_UNITS_32000_HZ;
      break;
    case 44100:
      units = MAD_UNITS_44100_HZ;
      break;
    case 48000:
      units = MAD_UNITS_48000_HZ;
      break;
    default: // By the MP3 specs, an MP3 _has_ to have one of the above samplerates...
      units = MAD_UNITS_44100_HZ;
#ifdef DEBUG
      cerr << "Warning: MP3 with corrupt samplerate, defaulting to 44100" << endl;
#endif
      setSamplerate (44100);
  }

  return (long unsigned) 2*mad_timer_count (filelength, units);
}

void TrackMp3::seek ( uint ms ) {
  if (!isValid() ) {
#ifdef DEBUG
    cerr << "seek failed: track not valid" << endl;
#endif
    return;
  }
  unsigned long long pos = (ms * samplerate() /* * channels()*/) / 1000;

  // Ensure that we are seeking to an even pos
  //Q_ASSERT(pos%2==0);
  MadSeekFrameType * cur;

  if (pos == 0) {
    // Seek to beginning of file
    // Re-init buffer:
    mad_stream_finish (&stream);
    mad_stream_init (&stream);
    mad_stream_options (&stream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer (&stream, (unsigned char *) inputbuf, inputbuf_len);
    mad_frame_init (frame);
    mad_synth_init (&synth);
    rest = -1;
    cur = m_qSeekList.at(0);
  } else {
    // Perform precise seek accomplished by using a frame in the seek list
    // Find the frame to seek to in the list
    int framePos = findFrame (pos);
    
    uint frameIdx = 10;//m_qSeekList.at();

    if (framePos == 0 || framePos > pos || frameIdx < 5) {
      // Re-init buffer:
      mad_stream_finish (&stream);
      mad_stream_init (&stream);
      mad_stream_options (&stream, MAD_OPTION_IGNORECRC);
      mad_stream_buffer (&stream, (unsigned char *) inputbuf, inputbuf_len);
      mad_frame_init (frame);
      mad_synth_init (&synth);
      rest = -1;
      cur = m_qSeekList.at(0);
    } else {
      // Start four frame before wanted frame to get in sync...
      cur = m_qSeekList[frameIdx-4];

      // Start from the new frame
      mad_stream_finish (&stream);
      mad_stream_init (&stream);
      mad_stream_options (&stream, MAD_OPTION_IGNORECRC);
      mad_stream_buffer (&stream, (const unsigned char *) cur->m_pStreamPos, inputbuf_len - (long int) (cur->m_pStreamPos - (unsigned char *) inputbuf) );
      mad_synth_mute (&synth);
      mad_frame_mute (frame);

      // Decode the three frames before
      mad_frame_decode (frame, &stream);
      mad_frame_decode (frame, &stream);
      mad_frame_decode (frame, &stream);
      if (mad_frame_decode (frame, &stream) ) cerr << "MP3 decode warning" << endl;
      mad_synth_frame (&synth, frame);

      // Set current position
      rest = -1;
      cur = m_qSeekList[frameIdx];
    }

    // synthesize the samples from the frame which should be discard to reach the requested position
    discard (pos - cur->pos);
  }

  // Unfortunately we don't know the exact position. The returned position is thus an
  // approximation only:
  m_iCurPosPCM = pos;
}

uint TrackMp3::currentPos() {
  if (isValid() ) {
    unsigned long long pos = 1000 * m_iCurPosPCM / (samplerate() /* *channels()*/);
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
int TrackMp3::readSamples ( SAMPLETYPE* buffer, int num ) {
  if (!isValid() ) return -1;

  // Ensure that we are reading an even number of samples. Otherwise this function may
  // go into an infinite loop
  assert (num % 2 == 0);
  unsigned nchannels = channels();
  unsigned nsamples = 0;
  short dest[num];
  short* destination = dest;

  // If samples are left from previous read, then copy them to start of destination
  if (rest > 0) {
    for (int i = rest; i < synth.pcm.length; i++) {
      // Left channel
      * (destination++) = madScale (synth.pcm.samples[0][i]);
      // Right channel
      if (nchannels > 1)
        * (destination++) = madScale (synth.pcm.samples[1][i]);
    }
    nsamples += nchannels * (synth.pcm.length - rest);
  }


  int no = 0;
  int frames = 0;
  while (nsamples < num) {
    if (mad_frame_decode (frame, &stream) ) {
      if (MAD_RECOVERABLE (stream.error) ) {
#ifdef DEBUG
        cerr << "MAD: Recoverable frame level ERR (" << mad_stream_errorstr (&stream) << ")" << endl;
#endif
        continue;
      } else if (stream.error == MAD_ERROR_BUFLEN) {
#ifdef DEBUG
        cerr << "MAD: buflen ERR" << endl;
#endif
        break;
      } else {
#ifdef DEBUG
        cerr << "MAD: Unrecoverable frame level ERR (" << mad_stream_errorstr (&stream) << ")";
#endif
        break;
      }
    }

    ++frames;

    /* Once decoded the frame is synthesized to PCM samples. No ERRs
     * are reported by mad_synth_frame();
     */
    mad_synth_frame (&synth, frame);

    // Number of channels in frame
    // ch = MAD_NCHANNELS(&frame->header);

    /* Synthesized samples must be converted from mad's fixed
     * point number to the consumer format (16 bit). Integer samples
     * are temporarily stored in a buffer that is flushed when
     * full.
     */

    // cerr << "synthlen " << Synth.pcm.length << ", remain " << (num - nsamples);
    no = math_min (synth.pcm.length, (num - nsamples) / 2);
    for (int i = 0; i < no; i++) {
      // Left channel
      * (destination++) = madScale (synth.pcm.samples[0][i]);

      // Right channel
      if (nchannels > 1)
        * (destination++) = madScale (synth.pcm.samples[1][i]);
    }
    nsamples += nchannels * no;

    // cerr << "decoded: " << nsamples << ", wanted: " << num;
  }

  // If samples are still left in buffer, set rest to the index of the unused samples
  if (synth.pcm.length > no) rest = no;
  else rest = -1;

  // convert the samples to float
  for (int i = 0; i < nsamples; ++i) {
    buffer[i] = (float) dest[i] / 32768.;
  }

  // cerr << "decoded " << Total_samples_decoded << " samples in " << frames << " frames, rest: " << rest << ", chan " << m_iChannels;
  m_iCurPosPCM += nsamples;
  return nsamples;
}

inline signed int TrackMp3::madScale (mad_fixed_t sample) {
  sample += (1L << (MAD_F_FRACBITS - 16) );

  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

// Decode the chosen number of samples and discard
unsigned long TrackMp3::discard(unsigned long samples_wanted) {
    unsigned long Total_samples_decoded = 0;
    int no;

    if(rest > 0)
        Total_samples_decoded += 2*(synth.pcm.length-rest);

    while (Total_samples_decoded < samples_wanted) {
        if(mad_frame_decode(frame,&stream)) {
            if(MAD_RECOVERABLE(stream.error)) {
                continue;
            } else if(stream.error==MAD_ERROR_BUFLEN) {
                break;
            } else {
                break;
            }
        }
        mad_synth_frame(&synth,frame);
        no = math_min(synth.pcm.length,(samples_wanted-Total_samples_decoded)/2);
        Total_samples_decoded += 2*no;
    }

    if (synth.pcm.length > no) rest = no;
    else rest = -1;


    return Total_samples_decoded;
}

int TrackMp3::findFrame(int pos) {
    // Guess position of frame in m_qSeekList based on average frame size
    uint frameIdx = math_min(m_qSeekList.size()-1, m_iAvgFrameSize ? (unsigned int)(pos/m_iAvgFrameSize) : 0);
    MadSeekFrameType* temp = m_qSeekList.at(frameIdx);

    // Ensure that the list element is not at a greater position than pos
    while (temp!=0 && temp->pos > pos) {
        temp = m_qSeekList.at(--frameIdx);
    }

    // Ensure that the following position is also not smaller than pos
    if (temp!=0) {
        while (temp!=0 && temp->pos < pos) {
            temp = m_qSeekList.at(++frameIdx);
        }

        if (temp==0)
            temp = m_qSeekList.back();
        else
            temp = m_qSeekList.at(--frameIdx);
    }

    if (temp>0) {
        return temp->pos;
    } else {
        return 0;
    }
}

void TrackMp3::storeBPM ( string format ) {
  string fname = filename();
  string sBPM = bpm2str ( getBPM(), format );
#ifdef HAVE_TAGLIB
  TagLib::MPEG::File f ( fname.c_str(), false );
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag (true);
  if (tag == NULL) {
    cerr << "BPM not saved !" << endl;
    return;
  }
  tag->removeFrames ("TBPM");                   // remove existing BPM frames
  TagLib::ID3v2::TextIdentificationFrame* bpmframe =
    new TagLib::ID3v2::TextIdentificationFrame ("TBPM", TagLib::String::Latin1);
  bpmframe->setText (sBPM.c_str() );
  tag->addFrame (bpmframe);                     // add new BPM frame
  f.save();                                     // save file
#elif defined(HAVE_ID3LIB)
  ID3_Tag tag ( fname.c_str() );                // Open file
  ID3_Frame* bpmframe = tag.Find ( ID3FID_BPM ); // find BPM frame
  if ( NULL != bpmframe )                       // if BPM frame found
    tag.RemoveFrame (bpmframe);                 // remove BPM frame
  ID3_Frame newbpmframe;                        // create BPM frame
  newbpmframe.SetID ( ID3FID_BPM );
  newbpmframe.Field ( ID3FN_TEXT ).Add ( sBPM.c_str() );
  tag.AddFrame ( newbpmframe );                 // add it to tag
  tag.Update (ID3TT_ID3V2);                     // save
#endif
}

void TrackMp3::readTags() {
  cerr << "TrackMp3 read tags" << endl;
  string fname = filename();
  string sbpm = "000.00";
#ifdef HAVE_TAGLIB
  TagLib::MPEG::File f (fname.c_str(), false);

  TagLib::ID3v2::Tag *tag = f.ID3v2Tag (false);
  if (tag != NULL) {
    setArtist (tag->artist().toCString() );
    setTitle (tag->title().toCString() );

    TagLib::List<TagLib::ID3v2::Frame*> lst = tag->frameList ("TBPM");
    if (lst.size() > 0) {
      TagLib::ID3v2::Frame* frame = lst[0];
      sbpm = frame->toString().toCString();
    }
  }
#elif defined(HAVE_ID3LIB)
  ID3_Tag tag ( fname.c_str() );
  if (char* sArtist = ID3_GetArtist (&tag) ) {
    setArtist (sArtist);
  }
  if (char* sTitle = ID3_GetTitle (&tag) ) {
    setTitle (sTitle);
  }

  ID3_Frame* bpmframe = tag.Find ( ID3FID_BPM );
  if ( NULL != bpmframe ) {
    ID3_Field* bpmfield = bpmframe->GetField (ID3FN_TEXT);
    if (NULL != bpmfield) {
      char buffer[1024];
      bpmfield->Get ( buffer, 1024 );
      sbpm = buffer;
    }
  }
#endif
  // set filename (without path) as title if the title is empty
  if (title().empty() )
    setTitle (fname.substr (fname.find_last_of ("/") + 1) );
  setBPM (str2bpm (sbpm) );
}

void TrackMp3::removeBPM() {
  string fname = filename();
#ifdef HAVE_TAGLIB
  TagLib::MPEG::File f ( fname.c_str(), false );
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag (true);
  if (tag == NULL) {
    return;
  }
  tag->removeFrames ("TBPM");
  f.save();
#endif
}
