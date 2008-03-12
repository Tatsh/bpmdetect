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

#include "track.h"

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

#include "BPMDetect.h"

#include <fmodex/fmod_errors.h>
#include <stdlib.h>

using namespace std;
using namespace soundtouch;

static double _dMinBPM = 80.;
static double _dMaxBPM = 185.;

// FIXME:
extern FMOD_SYSTEM* SoundSystem;
// Settings
extern bool force;

Track::Track(string filename) {
  setFilename(filename);
}

Track::~Track() {}

void Track::setMinBPM(double dMin) {
  if(dMin > 30.) _dMinBPM = dMin;
}

void Track::setMaxBPM(double dMax) {
  if(dMax < 300.) _dMaxBPM = dMax;
}

double Track::getMinBPM() {
  return _dMinBPM;
}

double Track::getMaxBPM() {
  return _dMaxBPM;
}

double Track::str2bpm( string sBPM ) {
  double BPM = 0;
  BPM = atof(sBPM.c_str());
  while( BPM > 250 ) BPM = BPM / 10;
  return BPM;
}

string Track::bpm2str( double dBPM, string format ) {
  #define MAX_LEN 10
  char buffer[MAX_LEN];

  if( format == "0.0" ) {
    snprintf(buffer, MAX_LEN, "%.1f", dBPM );
  } else if( format == "0" ) {
    snprintf(buffer, MAX_LEN, "%d", (int) dBPM );
  } else if( format == "000.00" ) {
    snprintf(buffer, MAX_LEN, "%06.2f", dBPM );
  } else if( format == "000.0" ) {
    snprintf(buffer, MAX_LEN, "%05.1f", dBPM );
  } else if( format == "000" ) {
    snprintf(buffer, MAX_LEN, "%03d", (int) dBPM );
  } else if( format == "00000" ) {
    snprintf(buffer, MAX_LEN, "%05d", (int) dBPM * 100. );
  } else { // all other formats are converted to "0.00"
    snprintf(buffer, MAX_LEN, "%.2f", dBPM );
  }

  string sBPM = buffer;
  return sBPM;
}

void Track::setFilename( string filename ) {
  if(filename == m_sFilename) return;
  // TODO: check for validity
  setValid(true);
  if(m_bValid) {
    setBPM(0);
    m_sFilename = filename;
  }
}

string Track::getFilename() const {
  return m_sFilename;
}

void Track::setValid(bool bValid) {
  m_bValid = bValid;
}

bool Track::isValid() const {
  return m_bValid;
}

void Track::setBPM(double dBPM) {
  m_dBPM = dBPM;
}

double Track::getBPM() const {
  return m_dBPM;
}

TrackType Track::getTrackType() {
  FMOD_SOUND *sound;
  FMOD_RESULT result;
  TrackType ttype = TYPE_UNKNOWN;
  string filename = getFilename();
  // open track with FMOD to get the type
  result = FMOD_System_CreateStream( SoundSystem, filename.c_str(),
                    FMOD_OPENONLY, 0, &sound );
  if ( result != FMOD_OK ) {
    return ttype;
  }

  FMOD_SOUND_TYPE type;
  FMOD_Sound_GetFormat ( sound, &type, 0, 0, 0 );
  FMOD_Sound_Release(sound);
  
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
  return ttype;
}

/**
 * @brief Save BPM to tag
 * @param format BPM format
 */
void Track::saveBPM( string format ) {
  string filename = getFilename();
  string sBPM = bpm2str( getBPM(), format );
  TrackType type = getTrackType();

  if ( type == TYPE_MPEG ) {
  //#ifdef HAVE_ID3LIB
    saveMPEG_ID3( sBPM, filename );
  //#elif defined(HAVE_TAGLIB)
    //saveMPEG_TAG( sBPM, filename );
  //#endif
  } else if ( type == TYPE_WAV ) {
  #ifdef HAVE_TAGLIB
    saveWAV_TAG( sBPM, filename );
  #elif defined(HAVE_ID3LIB)
    // saveWAV_ID3( sBPM, filename );
  #endif
  } else if ( type == TYPE_OGGVORBIS ) {
  #ifdef HAVE_TAGLIB
    saveOGG_TAG( sBPM, filename );
  #endif
  } else if ( type == TYPE_FLAC ) {
  #ifdef HAVE_TAGLIB
    saveFLAC_TAG( sBPM, filename );
  #endif
  } else {
  #ifdef DEBUG
    clog << "BPM not saved (file type not supported) !" << endl;
  #endif
  }
}


/// @brief Read ID3v2 tag (MPEG file)
taginfo_t Track::getTagInfoMPEG() {
  taginfo_t tagnfo;
  string filename = getFilename();
//#ifdef HAVE_ID3LIB
  ID3_Tag tag( filename.c_str() );
  tagnfo.Artist = ID3_GetArtist(&tag);
  tagnfo.Title  = ID3_GetTitle(&tag);

  string sbpm = "000.00";
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
  TagLib::MPEG::File f(filename, false);

  TagLib::ID3v2::Tag *tag = f.ID3v2Tag(false);
  if(tag != NULL) {
    tagnfo.Artist = tag->artist().toCString();
    tagnfo.Title = tag->title().toCString();

    TagLib::List<TagLib::ID3v2::Frame*> lst = tag->frameList("TBPM");
    if(lst.size() > 0) {
      TagLib::ID3v2::Frame* frame = lst[0];
      sbpm = frame->toString().toCString();
    }
  }
//#endif
*/
  // set filename (without path) as title if the title is empty
  if(tagnfo.Title.empty())
    tagnfo.Title = filename.substr(filename.find_last_of("/") + 1);
  tagnfo.BPM = bpm2str(str2bpm(sbpm), "000.00");

  return tagnfo;
}

/**
 * @brief Read ID3v2 tag (WAV file)
 * @param file file name
 */
taginfo_t Track::getTagInfoWAV() {
  taginfo_t tagnfo;
  string filename = getFilename();

#ifdef HAVE_TAGLIB
  TagLib::MPEG::File f(filename.c_str(), false);
  long pos = f.rfind("ID3", TagLib::File::End);
  if(pos < 0) pos = f.length();

  TagLib::ID3v2::Tag tag(&f, pos);
  tagnfo.Artist = tag.artist().toCString();
  tagnfo.Title  = tag.title().toCString();

  TagLib::List<TagLib::ID3v2::Frame*> lst = tag.frameList("TBPM");
  string sbpm = "000.00";
  if(lst.size() > 0) {
    TagLib::ID3v2::Frame* frame = lst[0];
    sbpm = frame->toString().toCString();
  }
#endif
  // set filename (without path) as title if the title is empty
  if(tagnfo.Title.empty())
    tagnfo.Title = filename.substr(filename.find_last_of("/") + 1);
  tagnfo.BPM = bpm2str(str2bpm(sbpm), "000.00");

  return tagnfo;
}


/**
 * @brief Correct BPM
 * if value is lower than min or higher than max
 * @return corrected BPM
 */
double Track::correctBPM( double dBPM ) {
  double min = getMinBPM();
  double max = getMaxBPM();

  if ( dBPM < 20. || dBPM > 500. )
    return 0.;

  while ( dBPM > max )
    dBPM /= 2.;
  while ( dBPM < min )
    dBPM *= 2.;

  return dBPM;
}

/**
 * @brief Print BPM to stdout
 * @param dBPM BPM to print
 * @param format BPM format
 */
void Track::printBPM( string format ) {
  cout << bpm2str(getBPM(), format) << " BPM" << endl;
}

/**
 * @brief Detect BPM of one track
 * @param filename is name of the file
 * @return detected BPM
 */
double Track::detectBPM( ) {
  FMOD_SOUND * sound;
  FMOD_RESULT result;
  FMOD_TAG tag;
  string filename = getFilename();

  result = FMOD_System_CreateStream( SoundSystem, filename.c_str(),
                        FMOD_OPENONLY, 0, &sound );
  if ( result != FMOD_OK ) {
    cerr << FMOD_ErrorString( result ) << endl;
    return 0;
  }

  float oldbpm = 0.0;
  if ( FMOD_Sound_GetTag( sound, "TBPM", 0, &tag ) == FMOD_OK ) {
    oldbpm = str2bpm(( char* ) tag.data);
  }
  if ( !force && oldbpm != 0 ) {
    return oldbpm;
  }

  {
#define CHUNKSIZE 4096
    SAMPLETYPE samples[ CHUNKSIZE / 2 ];
    unsigned int length = 0, read, totalsteps = 0;
    int channels = 2, bits = 16;
    float frequency = 44100;
    result = FMOD_Sound_GetLength( sound, &length, FMOD_TIMEUNIT_PCMBYTES );
    totalsteps = ( length / CHUNKSIZE );
    FMOD_Sound_GetDefaults( sound, &frequency, 0, 0, 0 );
    FMOD_Sound_GetFormat ( sound, 0, 0, &channels, &bits );
    if ( bits != 16 && bits != 8 ) {
      cerr << bits << " bit samples are not supported!" << endl;
      return 0;
    }

    BPMDetect bpmd( channels, ( int ) frequency );
    int cprogress = 0, pprogress = 0;
    do {
      if ( bits == 16 ) {
        int16_t data16[ CHUNKSIZE / 2 ];
        result = FMOD_Sound_ReadData( sound, data16, CHUNKSIZE, &read );
        for ( unsigned int i = 0; i < read / 2; i++ ) {
          samples[ i ] = ( float ) data16[ i ] / 32768;
        }
        bpmd.inputSamples( samples, read / ( 2 * channels ) );
      } else if ( bits == 8 ) {
        int8_t  data8[ CHUNKSIZE ];
        result = FMOD_Sound_ReadData( sound, data8, CHUNKSIZE, &read );
        for ( unsigned int i = 0; i < read; i++ ) {
          samples[ i ] = ( float ) data8[ i ] / 128;
        }
        bpmd.inputSamples( samples, read / channels );
      }
      cprogress++;
      while ( (100*cprogress/totalsteps) > pprogress ) {
        if( (++pprogress % 2) ) clog << ".";
      }
    } while ( result == FMOD_OK && read == CHUNKSIZE );
    FMOD_Sound_Release(sound); sound = 0;
    clog << endl;

    double BPM = bpmd.getBpm();
    BPM = correctBPM(BPM);
    setBPM(BPM);
    return BPM;
  }
}

//#ifdef HAVE_ID3LIB
/// Save BPM to ID3v2 tag using ID3 library
void Track::saveMPEG_ID3( string sBPM, string filename ) {
  ID3_Tag tag( filename.c_str() );              // Open file

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
void Track::saveWAV_ID3( string sBPM, string filename ) {
  ID3_Tag tag( filename.c_str() );              // Open file

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
void Track::saveMPEG_TAG( string sBPM, string filename ) {
  TagLib::MPEG::File f( filename.c_str(), false );
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
void Track::saveWAV_TAG( string sBPM, string filename ) {
  TagLib::MPEG::File f( filename.c_str(), false );
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
  f.save();
}

/// Save BPM to OGG file (xiphcomment)
void Track::saveOGG_TAG( string sBPM, string filename ) {
  TagLib::Ogg::Vorbis::File f( filename.c_str(), false );
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag == NULL) {
    cerr << "BPM not saved ! (failed)" << endl;
    return;
  }
  tag->addField("TBPM", sBPM.c_str(), true);    // add new BPM field (replace existing)
  f.save();
}

/// Save BPM to FLAC file (ID3v2 tag and xiphcomment)
void Track::saveFLAC_TAG( string sBPM, string filename ) {
  TagLib::FLAC::File f( filename.c_str(), false );
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
