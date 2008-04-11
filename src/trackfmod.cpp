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

#include "trackfmod.h"

#ifdef HAVE_TAGLIB
  #include <mpegfile.h>
  #include <vorbisfile.h>
  #include <flacfile.h>
  #include <id3v2tag.h>
  #include <id3v2frame.h>
  #include <textidentificationframe.h>
  #include <xiphcomment.h>
#endif   // HAVE_TAGLIB

// #ifdef HAVE_ID3LIB
  #ifdef _WIN32
  # define ID3LIB_LINKOPTION 1
  #endif
  #include <id3/tag.h>
  #include <id3/id3lib_streams.h>
  #include <id3/readers.h>
  #include <id3/misc_support.h>
// #endif   // HAVE_ID3LIB

#include <fmodex/fmod_errors.h>

using namespace std;
using namespace soundtouch;

static FMOD_SYSTEM* m_system;

bool TrackFMOD::initFMODSystem() {
  FMOD_RESULT result;
  unsigned int version;
  int numdrivers = 0;
#ifdef DEBUG
  clog << "Initializing FMOD sound system" << endl;
#endif
  if(m_system) return true;

  // create FMOD system
  result = FMOD_System_Create( &m_system );
  if( result != FMOD_OK ) {
    cerr << FMOD_ErrorString(result) << endl;
    return false;
  }
  // check FMOD version
  result = FMOD_System_GetVersion(m_system, &version);
#ifdef DEBUG
  if(result != FMOD_OK) {
    clog << "Can not get FMOD version" << endl;
  } else if (version < FMOD_VERSION) {
    fprintf(stderr, "Warning: You are using an old version of FMOD (%08x)."
                    "This program requires %08x\n", version, FMOD_VERSION);
  } else fprintf(stderr, "FMOD version: %08x\n", version);
#endif
  result = FMOD_System_GetNumDrivers(m_system, &numdrivers);
#ifdef DEBUG
  if(result != FMOD_OK) clog << "Can't get number of drivers" << endl;
#endif
  if(numdrivers > 0) {
    for( int i = 0; i < numdrivers; ++i ) {
      result = FMOD_System_SetDriver(m_system, i);
      if(result == FMOD_OK) {
        char name[256];
        #if (FMOD_VERSION >= 0x00041100)
          result = FMOD_System_GetDriverInfo(m_system, i, name, 256, 0);
        #else
          result = FMOD_System_GetDriverName(m_system, i, name, 256);
        #endif
        #ifdef DEBUG
        if(result == FMOD_OK) clog << "Driver: " << name << endl;
        #endif
        break;
      }
    }
  } else {
    cerr << "No soundcard found" << endl;
    cerr << FMOD_ErrorString(result) << endl;
    return false;
  }

  // init system
  result = FMOD_System_Init( m_system, 1, FMOD_INIT_NORMAL, 0 );
  if ( result == FMOD_OK ) {
    return true;
  } else {
    cerr << "System init: " << FMOD_ErrorString(result) << endl;
    return false;
  }
}

/// @brief Close FMOD sound system
void TrackFMOD::closeFMODSystem() {
  if(!m_system) return;
#ifdef DEBUG
  clog << "Closing FMOD sound system" << endl;
#endif
  FMOD_CHANNELGROUP* masterchgrp = 0;
  if(FMOD_OK == FMOD_System_GetMasterChannelGroup(m_system, &masterchgrp)) {
    FMOD_ChannelGroup_Stop(masterchgrp);
    FMOD_ChannelGroup_SetVolume(masterchgrp, 0.0);
    FMOD_ChannelGroup_Release(masterchgrp);
    masterchgrp = 0;
  }
  FMOD_System_Close(m_system);
  FMOD_System_Release(m_system);
  m_system = 0;
}

FMOD_SYSTEM* TrackFMOD::getFMODSystem() {
  return m_system;
}

TrackFMOD::TrackFMOD( const char* fname, bool readtags ) : Track() {
  m_sound = 0;
  m_iCurPosBytes = 0;
  setFilename( fname, readtags );
}

TrackFMOD::TrackFMOD( string fname, bool readtags ) : Track() {
  m_sound = 0;
  m_iCurPosBytes = 0;
  setFilename( fname, readtags );
}

TrackFMOD::~TrackFMOD() {
  close();
}

void TrackFMOD::open() {
  if(isValid()) close();
  FMOD_RESULT result;
  m_iCurPosBytes = 0;
  string fname = filename();
  result = FMOD_System_CreateStream( m_system,
               fname.c_str(), FMOD_OPENONLY, 0, &m_sound );
  if ( result != FMOD_OK ) {
    init();
    m_sound = 0;
    return;
  }

  FMOD_SOUND_TYPE type;
  int ttype = TYPE_UNKNOWN;
  int channels = 0, 
      bits = 0;
  float frequency = 0;
  uint len;
  FMOD_Sound_GetLength( m_sound, &len, FMOD_TIMEUNIT_MS );
  setLength( len );
  setStartPos( 0 );
  setEndPos( len );
  setValid(true);

  FMOD_Sound_GetDefaults( m_sound, &frequency, 0, 0, 0 );
  FMOD_Sound_GetFormat ( m_sound, &type, 0, &channels, &bits );

  setSamplerate( (int) frequency);
  setSampleBytes(bits / 8);
  setChannels(channels);
  switch( type ) {
    case FMOD_SOUND_TYPE_MPEG:
      ttype = TYPE_MPEG;
      break;
    case FMOD_SOUND_TYPE_WAV:
      ttype = TYPE_WAV;
      break;
    case FMOD_SOUND_TYPE_OGGVORBIS:
      ttype = TYPE_OGGVORBIS;
      break;
    case FMOD_SOUND_TYPE_FLAC:
      ttype = TYPE_FLAC;
      break;
    default:
      ttype = TYPE_UNKNOWN;
      break;
  }
  setTrackType((TRACKTYPE) ttype);
}

void TrackFMOD::close() {
  if(m_sound) FMOD_Sound_Release(m_sound);
  m_sound = 0;
  m_iCurPosBytes = 0;
}

void TrackFMOD::seek( uint ms ) {
  if(isValid()) {
    uint pos = (ms * samplerate() * sampleBytes()) / 1000;
    FMOD_RESULT res = FMOD_Sound_SeekData(m_sound, pos);
    if(res == FMOD_OK) {
      m_iCurPosBytes = pos;
    }
#ifdef DEBUG
    else {
      cerr << "seek failed: " << FMOD_ErrorString(res) << endl;
    }
  } else {
    cerr << "seek failed: track not valid" << endl;
#endif
  }
}

uint TrackFMOD::currentPos() {
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
int TrackFMOD::readSamples( SAMPLETYPE* buffer, int num ) {
  if(!isValid()) return -1;

  FMOD_RESULT result;
  uint readbytes = 0;
  int sbytes = sampleBytes();
  uint bytes = num * sbytes;

  if(sbytes == 2) {
    int16_t data[bytes/2];
    result = FMOD_Sound_ReadData( m_sound, data, bytes, &readbytes );
    if(!result == FMOD_OK) return -1;
    for ( uint i = 0; i < readbytes/sbytes; ++i )
      buffer[i] = (float) data[i] / 32768;
  } else if(sbytes == 1) {
    int8_t data[bytes];
    result = FMOD_Sound_ReadData( m_sound, data, bytes, &readbytes );
    if(!result == FMOD_OK) return -1;
    for ( uint i = 0; i < (readbytes); ++i )
      buffer[i] = (float) data[i] / 128;
  }

  m_iCurPosBytes += readbytes;
  return readbytes / sbytes ;
}

/**
 * @brief Save BPM to tag
 * @param format BPM format
 */
void TrackFMOD::storeBPM( string format ) {
  string fname = filename();
  string sBPM = bpm2str( getBPM(), format );
  int type = trackType();

  if ( type == TYPE_MPEG ) {
  //#ifdef HAVE_ID3LIB
    saveMPEG_ID3( sBPM );
  //#elif defined(HAVE_TAGLIB)
    //saveMPEG_TAG( sBPM );
  //#endif
  } else if ( type == TYPE_WAV ) {
  #ifdef HAVE_TAGLIB
    saveWAV_TAG( sBPM );
  #elif defined(HAVE_ID3LIB)
    // saveWAV_ID3( sBPM );
  #endif
  } else if ( type == TYPE_OGGVORBIS ) {
  #ifdef HAVE_TAGLIB
    saveOGG_TAG( sBPM );
  #endif
  } else if ( type == TYPE_FLAC ) {
  #ifdef HAVE_TAGLIB
    saveFLAC_TAG( sBPM );
  #endif
  } else {
  #ifdef DEBUG
    clog << "BPM not saved (file type not supported) !" << endl;
  #endif
  }
}

void TrackFMOD::readTags() {
  int type = trackType();

  if ( type == TYPE_MPEG ) {
    readTagsMPEG();
  } else if ( type == TYPE_WAV ) {
    readTagsWAV();
  } else if ( type == TYPE_OGGVORBIS ) {
    readTagsOGG();
  } else if ( type == TYPE_FLAC ) {
    readTagsFLAC();
  } else {
  #ifdef DEBUG
    clog << "Reading tags: file type not supported" << endl;
  #endif
  }
}

void TrackFMOD::readTagsMPEG() {
  string sbpm = "000.00";
  string fname = filename();
//#ifdef HAVE_ID3LIB
  ID3_Tag tag( fname.c_str() );
  if(char* sArtist = ID3_GetArtist(&tag)) {
    setArtist(sArtist);
  }
  if(char* sTitle = ID3_GetTitle(&tag)) {
    setTitle(sTitle);
  }

  ID3_Frame* bpmframe = tag.Find( ID3FID_BPM );
  if ( NULL != bpmframe ) {
    ID3_Field* bpmfield = bpmframe->GetField(ID3FN_TEXT);
    if(NULL != bpmfield) {
      char buffer[1024];
      bpmfield->Get( buffer, 1024 );
      sbpm = buffer;
    }
  }

/*
//#elif defined(HAVE_TAGLIB)
  TagLib::MPEG::File f(fname(), false);

  TagLib::ID3v2::Tag *tag = f.ID3v2Tag(false);
  if(tag != NULL) {
    setArtist(tag->artist().toCString());
    setTitle(tag->title().toCString());

    TagLib::List<TagLib::ID3v2::Frame*> lst = tag->frameList("TBPM");
    if(lst.size() > 0) {
      TagLib::ID3v2::Frame* frame = lst[0];
      sbpm = frame->toString().toCString();
    }
  }
//#endif
*/
  // set filename (without path) as title if the title is empty
  if(title().empty())
    setTitle(fname.substr(fname.find_last_of("/") + 1));
  setBPM(str2bpm(sbpm));
}

void TrackFMOD::readTagsWAV() {
  string sbpm = "000.00";
  string fname = filename();
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
}

void TrackFMOD::readTagsOGG() {
  string sbpm = "000.00";
  string fname = filename();
#ifdef HAVE_TAGLIB
  TagLib::Ogg::Vorbis::File f( fname.c_str(), false );
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag != NULL) {
    setArtist(tag->artist().toCString());
    setTitle(tag->title().toCString());
    TagLib::Ogg::FieldListMap flmap = tag->fieldListMap();
    TagLib::StringList strl = flmap["TBPM"];
    if(!strl.isEmpty()) sbpm = strl[0].toCString();
  }
#endif
  if(title().empty()) 
    setTitle(fname.substr(fname.find_last_of("/") + 1));
  setBPM(str2bpm(sbpm));
}

void TrackFMOD::readTagsFLAC() {
  string sbpm = "000.00";
  string fname = filename();
#ifdef HAVE_TAGLIB
  TagLib::FLAC::File f( fname.c_str(), false );
  TagLib::Tag* tag = f.tag();
  if(tag != NULL) {
    setArtist(tag->artist().toCString());
    setTitle(tag->title().toCString());
  }

  TagLib::Ogg::XiphComment* xiph = f.xiphComment(true);
  if(xiph != NULL) {
    TagLib::Ogg::FieldListMap flmap = xiph->fieldListMap();
    TagLib::StringList strl = flmap["TBPM"];
    if(!strl.isEmpty()) sbpm = strl[0].toCString();
    else {
      TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
      if(tag != NULL) {
        TagLib::List<TagLib::ID3v2::Frame*> lst = tag->frameList("TBPM");
        if(lst.size() > 0) {
          TagLib::ID3v2::Frame* frame = lst[0];
          sbpm = frame->toString().toCString();
        }
      }
    }
  }
#endif
  if(title().empty()) 
    setTitle(fname.substr(fname.find_last_of("/") + 1));
  setBPM(str2bpm(sbpm));
}

//#ifdef HAVE_ID3LIB
/// Save BPM to ID3v2 tag using ID3 library
void TrackFMOD::saveMPEG_ID3( string sBPM ) {
  ID3_Tag tag( filename().c_str() );            // Open file

  ID3_Frame* bpmframe = tag.Find( ID3FID_BPM ); // find BPM frame

  if ( NULL != bpmframe )                       // if BPM frame found
    tag.RemoveFrame(bpmframe);                  // remove BPM frame

  ID3_Frame newbpmframe;                        // create BPM frame
  newbpmframe.SetID( ID3FID_BPM );
  newbpmframe.Field( ID3FN_TEXT ).Add( sBPM.c_str() );
  tag.AddFrame( newbpmframe );                  // add it to tag
  tag.Update(ID3TT_ID3V2);                      // save
}

/// Save BPM to ID3v2 tag using ID3 library
void TrackFMOD::saveWAV_ID3( string sBPM ) {
  ID3_Tag tag( filename().c_str() );            // Open file

  ID3_Frame* bpmframe = tag.Find( ID3FID_BPM ); // find BPM frame

  if ( NULL != bpmframe )                       // if BPM frame found
    tag.RemoveFrame(bpmframe);                  // remove BPM frame

  ID3_Frame newbpmframe;                        // create BPM frame
  newbpmframe.SetID( ID3FID_BPM );
  newbpmframe.Field( ID3FN_TEXT ).Add( sBPM.c_str() );
  tag.AddFrame( newbpmframe );                  // add it to tag
  tag.Update(ID3TT_ID3V2);                      // save
}
//#endif // HAVE_ID3LIB

#ifdef HAVE_TAGLIB
/// Save BPM to MPEG file (ID3v2 tag)
void TrackFMOD::saveMPEG_TAG( string sBPM ) {
  TagLib::MPEG::File f( filename().c_str(), false );
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag == NULL) {
    cerr << "BPM not saved !" << endl;
    return;
  }
  tag->removeFrames("TBPM");                    // remove existing BPM frames
  TagLib::ID3v2::TextIdentificationFrame* bpmframe =
      new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
  bpmframe->setText(sBPM.c_str());
  tag->addFrame(bpmframe);                      // add new BPM frame
  f.save();                                     // save file
}

/// Save BPM to WAV file (ID3v2 tag)
void TrackFMOD::saveWAV_TAG( string sBPM ) {
/*
  TagLib::MPEG::File f( filename().c_str(), false );
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
}

/// Save BPM to OGG file (xiphcomment)
void TrackFMOD::saveOGG_TAG( string sBPM ) {
  TagLib::Ogg::Vorbis::File f( filename().c_str(), false );
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag == NULL) {
    cerr << "BPM not saved ! (failed)" << endl;
    return;
  }
  tag->addField("TBPM", sBPM.c_str(), true);    // add new BPM field (replace existing)
  f.save();
}

/// Save BPM to FLAC file (ID3v2 tag and xiphcomment)
void TrackFMOD::saveFLAC_TAG( string sBPM ) {
  TagLib::FLAC::File f( filename().c_str(), false );
  TagLib::Ogg::XiphComment* xiph = f.xiphComment(true);
  if(xiph != NULL) {
    xiph->addField("TBPM", sBPM.c_str(), true);  // add new BPM field (replace existing)
  }

  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag != NULL) {
    tag->removeFrames("TBPM");                  // remove existing BPM frames
    TagLib::ID3v2::TextIdentificationFrame* bpmframe =
        new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
    bpmframe->setText(sBPM.c_str());
    tag->addFrame(bpmframe);                    // add new BPM frame
  }

  f.save();
}
#endif // HAVE_TAGLIB

void TrackFMOD::clearBPMMPEG() {
#ifdef HAVE_TAGLIB
  TagLib::MPEG::File f( filename().c_str(), false );
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag == NULL) {
    return;
  }
  tag->removeFrames("TBPM");
  f.save();
#endif
}

void TrackFMOD::clearBPMWAV() {
#ifdef HAVE_TAGLIB
/*
  TagLib::MPEG::File f( filename().c_str(), false );
  long offset = f.rfind("ID3", TagLib::File::End);
  if(offset < 0) offset = f.length();           // ID3 tag offset
  TagLib::ID3v2::Tag tag(&f, offset);
  tag.removeFrames("TBPM");
  TagLib::ByteVector tdata = tag.render();
  f.seek(offset);
  f.writeBlock(tdata);
  //f.save();
*/
#endif
}

void TrackFMOD::clearBPMOGG() {
#ifdef HAVE_TAGLIB
  TagLib::Ogg::Vorbis::File f( filename().c_str(), false );
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag == NULL) {
    return;
  }
  tag->removeField("TBPM");
  f.save();
#endif
}

void TrackFMOD::clearBPMFLAC() {
#ifdef HAVE_TAGLIB
  TagLib::FLAC::File f( filename().c_str(), false );
  TagLib::Ogg::XiphComment* xiph = f.xiphComment(true);
  if(xiph != NULL) {
    xiph->removeField("TBPM");
  }

  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag != NULL) {
    tag->removeFrames("TBPM");
  }

  f.save();
#endif
}

void TrackFMOD::removeBPM() {
  int type = trackType();

  if ( type == TYPE_MPEG ) {
    clearBPMMPEG();
  } else if ( type == TYPE_WAV ) {
    clearBPMWAV();
  } else if ( type == TYPE_OGGVORBIS ) {
    clearBPMOGG();
  } else if ( type == TYPE_FLAC ) {
    clearBPMFLAC();
  } else {
  #ifdef DEBUG
    clog << "Clear BPM: file type not supported" << endl;
  #endif
  }
}
