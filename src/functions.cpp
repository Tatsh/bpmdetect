/***************************************************************************
     Copyright          : (C) 2007 by Martin Sakmar
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

#ifndef NO_GUI
# include <qvariant.h>
#endif
#include "functions.h"

// #ifdef HAVE_TAGLIB
#include <mpegfile.h>
#include <vorbisfile.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <textidentificationframe.h>
#include <xiphcomment.h>
// #endif   // HAVE_TAGLIB


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
#include <math.h>
#include <stdlib.h>

#ifdef NO_GUI

#endif



/// Save BPM to ID3v2 tag using ID3 library
static void ID3SaveMPEG(string sBPM, string filename) {
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
static void ID3SaveWAV(string sBPM, string filename) {
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

/// @brief Save BPM to MPEG file (ID3v2 tag)
static void TAGSaveMPEG(string sBPM, string filename) {
  TagLib::MPEG::File f(filename.c_str(), false);
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

/// @brief Save BPM to WAV file (ID3v2 tag)
static void TAGSaveWAV(string sBPM, string filename) {
  TagLib::MPEG::File f(filename.c_str(), false);
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

/// @brief Save BPM to OGG file (xiphcomment)
static void TAGSaveOGG(string sBPM, string filename) {
  TagLib::Ogg::Vorbis::File f(filename.c_str(), false);
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag == NULL) {
    cerr << "BPM not saved ! (failed)" << endl;
    return;
  }
  tag->addField("TBPM", sBPM.c_str(), true);    // add new BPM field (replace existing)
  f.save();
}

/// @brief Save BPM to FLAC file (ID3v2 tag and xiphcomment)
static void TAGSaveFLAC(string sBPM, string filename) {
  TagLib::FLAC::File f(filename.c_str(), false);
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

/**
 * @brief Save BPM to file
 * @param filename track filename
 * @param dBPM BPM to save
 */
void saveBPM( string filename, double dBPM, string format ) {
  FMOD_SOUND *sound;
  FMOD_RESULT result;
  // open track with FMOD to get the type
  result = FMOD_System_CreateStream( SoundSystem, filename.c_str(),
                    FMOD_OPENONLY, 0, &sound );
  if ( result != FMOD_OK ) {
    cerr << "Save BPM: " << FMOD_ErrorString( result ) << " : " << filename << endl;
    return;
  }

  FMOD_SOUND_TYPE type;
  FMOD_Sound_GetFormat ( sound, &type, 0, 0, 0 );
  FMOD_Sound_Release(sound);

  string BPM = bpm2str( dBPM, format );

  if ( type == FMOD_SOUND_TYPE_MPEG ) {
    // taglib
    //TAGSaveMPEG( BPM, filename);
    // ID3 lib
    ID3SaveMPEG( BPM, filename );
  } else if ( type == FMOD_SOUND_TYPE_WAV ) {
    // taglib
    TAGSaveWAV( BPM, filename );
    // ID3 lib
    // ID3SaveWav( BPM, filename );
  } else if ( type == FMOD_SOUND_TYPE_OGGVORBIS ) {
    // taglib
    TAGSaveOGG( BPM, filename );
  } else if ( type == FMOD_SOUND_TYPE_FLAC ) {
    // taglib
    TAGSaveFLAC( BPM, filename );
  } else {
  #ifdef DEBUG
    clog << "BPM not saved !" << endl;
  #endif
  }
}


/// @brief Read ID3v2 tag (MPEG file)
taginfo_t getTagInfoMPEG(string filename) {
  taginfo_t tagnfo;

  // id3lib
  ID3_Tag tag( filename.c_str() );
  if(char* sArtist = ID3_GetArtist(&tag)) {
    tagnfo.Artist = sArtist;
  }
  if(char* sTitle = ID3_GetTitle(&tag)) {
    tagnfo.Title = sTitle;
  }

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
  // taglib
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
taginfo_t getTagInfoWAV(string filename) {
  taginfo_t tagnfo;

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

  // set filename (without path) as title if the title is empty
  if(tagnfo.Title.empty())
    tagnfo.Title = filename.substr(filename.find_last_of("/") + 1);
  tagnfo.BPM = bpm2str(str2bpm(sbpm), "000.00");

  return tagnfo;
}


/**
 * @brief Correct BPM
 * if value is lower than min or higher than max
 * @param BPM BPM to correct
 * @param min minimum BPM
 * @param max maximum BPM
 * @return corrected BPM
 */
double correctBPM( double dBPM, double min, double max ) {
  if ( dBPM < 10. && dBPM > 1000. )
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
void printBPM( double dBPM, string format ) {
  cout << bpm2str(dBPM, format) << " BPM" << endl;
}

/**
 * @brief Detect BPM of one track
 * @param filename is name of the file
 * @return detected BPM
 */
double Detect_BPM( string filename ) {
  Track trk(filename);
  return trk.detectBPM();
}

double str2bpm( string sBPM ) {
  return Track::str2bpm(sBPM);
}

string bpm2str( double dBPM, string format ) {
  return Track::bpm2str(dBPM, format);
}


