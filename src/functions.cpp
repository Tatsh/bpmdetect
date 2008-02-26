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

#include "functions.h"

#define MINIMUM_BPM 80.
#define MAXIMUM_BPM 185.

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

#include <math.h>
#include <qregexp.h>

// Settings
QString set_format;
bool    set_skip;
bool    set_save;


#ifdef NO_GUI

#endif

/**
 * @brief Create and init FMOD sound system
 * @return true if initialized successfully
 * @return false on error
 */
bool Init_FMOD_System() {
  FMOD_RESULT result;
  unsigned int version;
  int numdrivers = 0;

  qDebug("Initializing FMOD sound system");
  if(SoundSystem) return true;

  // create FMOD system
  result = FMOD_System_Create( &SoundSystem );
  if( result != FMOD_OK ) {
    qDebug(FMOD_ErrorString(result));
    return false;
  }
  // check FMOD version
  result = FMOD_System_GetVersion(SoundSystem, &version);
  if(result != FMOD_OK) {
    qDebug("Can not get FMOD version");
  } else if (version < FMOD_VERSION) {
    qDebug("You are using an old version of FMOD %08x.  This program requires %08x", version, FMOD_VERSION);
  } else qDebug("FMOD version %08x.", version);
  // get number of drivers
  result = FMOD_System_GetNumDrivers(SoundSystem, &numdrivers);
  if(result != FMOD_OK) qDebug("Can't get number of drivers");
  // set driver
  if(numdrivers > 0) {
    for( int i = 0; i < numdrivers; ++i ) {
      result = FMOD_System_SetDriver(SoundSystem, i);
      if(result == FMOD_OK) {
        char name[256];
        result = FMOD_System_GetDriverInfo(SoundSystem, i, name, 256, 0);
        if(result == FMOD_OK) qDebug("Driver: %s", name);
        break;
      }
    }
  } else {
    qDebug("Error: No soundcard found");
    qDebug(FMOD_ErrorString(result));
    return false;
  }

  // init system
  result = FMOD_System_Init( SoundSystem, 3, FMOD_INIT_NORMAL, 0 );
  if ( result == FMOD_OK ) {
    return true;
  } else {
    qDebug(FMOD_ErrorString(result));
    return false;
  }
}

/// @brief Close FMOD sound system
void Close_FMOD_System() {
  if(!SoundSystem) return;

  qDebug("Closing FMOD sound system");
  FMOD_CHANNELGROUP* masterchgrp = 0;
  if(FMOD_OK == FMOD_System_GetMasterChannelGroup(SoundSystem, &masterchgrp)) {
    FMOD_ChannelGroup_Stop(masterchgrp);
    FMOD_ChannelGroup_SetVolume(masterchgrp, 0.0);
    FMOD_ChannelGroup_Release(masterchgrp);
    masterchgrp = 0;
  }
  FMOD_System_Close(SoundSystem);
  FMOD_System_Release(SoundSystem);
  SoundSystem = 0;
}


/// Save BPM to ID3v2 tag using ID3 library
static void ID3SaveMPEG(QString BPM, QString file) {
  ID3_Tag tag( file.local8Bit() );              // Open file

  ID3_Frame* bpmframe = tag.Find( ID3FID_BPM ); // find BPM frame

  if ( NULL != bpmframe )                       // if BPM frame found
    tag.RemoveFrame(bpmframe);                  // remove BPM frame

  ID3_Frame newbpmframe;                        // create BPM frame
  newbpmframe.SetID( ID3FID_BPM );
  newbpmframe.Field( ID3FN_TEXT ).Add( BPM.ascii() );
  tag.AddFrame( newbpmframe );                  // add it to tag
  tag.Update(ID3TT_ID3V2);                      // save
}

/// Save BPM to ID3v2 tag using ID3 library
static void ID3SaveWAV(QString BPM, QString file) {
  ID3_Tag tag( file.local8Bit() );              // Open file

  ID3_Frame* bpmframe = tag.Find( ID3FID_BPM ); // find BPM frame

  if ( NULL != bpmframe )                       // if BPM frame found
    tag.RemoveFrame(bpmframe);                  // remove BPM frame

  ID3_Frame newbpmframe;                        // create BPM frame
  newbpmframe.SetID( ID3FID_BPM );
  newbpmframe.Field( ID3FN_TEXT ).Add( BPM.ascii() );
  tag.AddFrame( newbpmframe );                  // add it to tag
  tag.Update(ID3TT_ID3V2);                      // save
}


/// @brief Save BPM to MPEG file (ID3v2 tag)
static void TAGSaveMPEG(QString BPM, QString file) {
  TagLib::MPEG::File f(file.local8Bit(), false);// open file
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag == NULL) {
    cerr << "BPM not saved !" << endl;
    return;
  }
  tag->removeFrames("TBPM");                    // remove existing BPM frames
  TagLib::ID3v2::TextIdentificationFrame* bpmframe =
      new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
  bpmframe->setText(BPM.ascii());
  tag->addFrame(bpmframe);                      // add new BPM frame
  f.save();                                     // save file
}

/// @brief Save BPM to WAV file (ID3v2 tag)
static void TAGSaveWAV(QString BPM, QString file) {
  TagLib::MPEG::File f(file.local8Bit(), false);// open file
  long offset = f.rfind("ID3", TagLib::File::End);// ID3 tag offset
  if(offset < 0) offset = f.length();
  TagLib::ID3v2::Tag tag(&f, offset);
  tag.removeFrames("TBPM");                     // remove existing BPM frames

  TagLib::ID3v2::TextIdentificationFrame* bpmframe =
      new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
  bpmframe->setText(BPM.ascii());
  tag.addFrame(bpmframe);                       // add new BPM frame

  TagLib::ByteVector tdata = tag.render();      // render tag to binary data
  f.seek(offset);
  f.writeBlock(tdata);                          // write to file
  f.save();
}

/// @brief Save BPM to OGG file (xiphcomment)
static void TAGSaveOGG(QString BPM, QString file) {
  TagLib::Ogg::Vorbis::File f(file.local8Bit(), false);
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag == NULL) {
    cerr << "BPM not saved ! (failed)" << endl;
    return;
  }
  tag->addField("TBPM", BPM.ascii(), true); // add new BPM field (replace existing)
  f.save();
}

/// @brief Save BPM to FLAC file (ID3v2 tag and xiphcomment)
static void TAGSaveFLAC(QString BPM, QString file) {
  TagLib::FLAC::File f(file.local8Bit(), false);
  // XiphComment
  TagLib::Ogg::XiphComment* xiph = f.xiphComment(true);
  if(xiph == NULL) {
    cerr << "BPM not saved to XiphComment ! (failed)" << endl;
    return;
  }
  xiph->addField("TBPM", BPM.ascii(), true); // add new BPM field (replace existing)

  // ID3v2 Tag
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag == NULL) {
    cerr << "BPM not saved to ID3v2 tag ! (failed)" << endl;
    return;
  }
  tag->removeFrames("TBPM");                    // remove existing BPM frames
  TagLib::ID3v2::TextIdentificationFrame* bpmframe =
      new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);
  bpmframe->setText(BPM.ascii());
  tag->addFrame(bpmframe);                      // add new BPM frame
  f.save();
}

/**
 * @brief Save BPM to file
 * @param file name of the file
 * @param fBPM BPM to save
 */
void Save_BPM( QString file, float fBPM ) {
  QFile f( file );
  if ( ! f.exists() )
    return ;

  FMOD_SOUND *sound;
  FMOD_RESULT result;
  result = FMOD_System_CreateStream( SoundSystem, file.local8Bit(),
                    FMOD_OPENONLY | FMOD_ACCURATETIME, 0, &sound );
  if ( result != FMOD_OK ) {
    cout << FMOD_ErrorString( result ) << " : " << file.local8Bit() << endl;
    return ;
  }

  FMOD_SOUND_TYPE type;
  FMOD_Sound_GetFormat ( sound, &type, 0, 0, 0 );
  FMOD_Sound_Release(sound);

  QString BPM = Format_BPM( fBPM );

  if ( type == FMOD_SOUND_TYPE_MPEG ) {
    // taglib
    //TAGSaveMPEG( BPM, file);
    // ID3 lib
    ID3SaveMPEG( BPM, file );
  } else if ( type == FMOD_SOUND_TYPE_WAV ) {
    // taglib
    TAGSaveWAV( BPM, file );
    // ID3 lib
    // ID3SaveWav( BPM, file );
  } else if ( type == FMOD_SOUND_TYPE_OGGVORBIS ) {
    // taglib
    TAGSaveOGG( BPM, file );
  } else if ( type == FMOD_SOUND_TYPE_FLAC ) {
    // taglib
    TAGSaveFLAC( BPM, file );
  } else {
    cerr << "BPM not saved !" << endl;
  }
}

/**
 * @brief Read ID3v2 tag (MPEG file)
 * @param file file name
 */
TAGINFO GetTagInfoMPEG(QString file) {
  TAGINFO t;
  QString s;

  // id3lib
  ID3_Tag tag( file.local8Bit() );
  t.Artist = QString::fromLocal8Bit(ID3_GetArtist(&tag));
  t.Title = QString::fromLocal8Bit(ID3_GetTitle(&tag));
  ID3_Frame* bpmframe = tag.Find( ID3FID_BPM );

  if ( NULL != bpmframe ) {
    ID3_Field* bpmfield = bpmframe->GetField(ID3FN_TEXT);
    if(NULL != bpmfield) {
      char buffer[1024];
      bpmfield->Get( buffer, 1024 );
      s = buffer;
    }
  }

/*
  // taglib
  TagLib::MPEG::File f(file, false);

  TagLib::ID3v2::Tag *tag = f.ID3v2Tag(false);
  if(tag != NULL) {
    t.Artist = tag->artist().toCString();
    t.Title = tag->title().toCString();

    TagLib::List<TagLib::ID3v2::Frame*> lst = tag->frameList("TBPM");
    if(lst.size() > 0) {
      TagLib::ID3v2::Frame* frame = lst[0];
      s = frame->toString().toCString();
    }
  }
*/
  t.BPM = BPM2str(Str2BPM(s));
  if(t.Title.isEmpty())
    t.Title = file.right(file.length() - file.findRev("/") - 1);

  return t;
}

/**
 * @brief Read ID3v2 tag (WAV file)
 * @param file file name
 */
TAGINFO GetTagInfoWAV(QString file) {
  TAGINFO t;

  TagLib::MPEG::File f(file, false);
  long pos = f.rfind("ID3", TagLib::File::End);
  if(pos < 0) pos = f.length();

  TagLib::ID3v2::Tag tag(&f, pos);
  t.Artist = tag.artist().toCString();
  t.Title = tag.title().toCString();

  TagLib::List<TagLib::ID3v2::Frame*> lst = tag.frameList("TBPM");
  QString s;
  if(lst.size() > 0) {
    TagLib::ID3v2::Frame* frame = lst[0];
    s = frame->toString().toCString();
  }

  if(t.Title.isEmpty())
    t.Title = file.right(file.length() - file.findRev("/") - 1);
  t.BPM = BPM2str(Str2BPM(s));

  return t;
}


/**
 * @brief Correct BPM
 * if value is lower than MINIMUM_BPM or higher than MAXIMUM_BPM
 * @param BPM BPM to correct
 * @return corrected BPM
 */
float Correct_BPM( float BPM ) {
  if ( BPM == 0. )
    return BPM;

  while ( BPM > MAXIMUM_BPM )
    BPM /= 2;
  while ( BPM < MINIMUM_BPM )
    BPM *= 2;

  return BPM;
}

/**
 * @brief Print BPM to stdout
 * @param BPM BPM to print
 */
void Print_BPM( float BPM ) {
  QString sbpm;
  sbpm.sprintf( "%.2f", BPM );
  sbpm = sbpm.rightJustify( 6, '0' );
  cout << sbpm << " BPM found" << endl << endl;
}

/**
 * @brief Detect BPM of one track
 * @param filename is name of the file
 */
void Detect_BPM( QString filename ) {
  FMOD_SOUND * sound;
  FMOD_RESULT result;
  FMOD_TAG tag;

  result = FMOD_System_CreateStream( SoundSystem, filename.local8Bit(),
                        FMOD_OPENONLY | FMOD_ACCURATETIME, 0, &sound );
  if ( result != FMOD_OK ) {
    cerr << FMOD_ErrorString( result ) << endl;
    Print_BPM(0);
    return;
  }

  float oldbpm = 0.0;
  if ( FMOD_Sound_GetTag( sound, "TBPM", 0, &tag ) == FMOD_OK ) {
    QString s = ( char* ) tag.data;
    oldbpm = s.toFloat() / 100. ;
  }
  if ( !force && oldbpm != 0 ) {
    Print_BPM( oldbpm );
    return ;
  }

  {
#define CHUNKSIZE 4096
    int16_t data16[ CHUNKSIZE / 2 ];  // for 16 bit samples
    int8_t  data8[ CHUNKSIZE ];       // for 8 bit samples
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
      Print_BPM( 0 );
      return ;
    }

    BPMDetect bpmd( channels, ( int ) frequency );
    int cprogress = 0;
    do {
      if ( bits == 16 ) {
        result = FMOD_Sound_ReadData( sound, data16, CHUNKSIZE, &read );
        for ( unsigned int i = 0; i < read / 2; i++ ) {
          samples[ i ] = ( float ) data16[ i ] / 32768;
        }
        bpmd.inputSamples( samples, read / ( 2 * channels ) );
      } else if ( bits == 8 ) {
        result = FMOD_Sound_ReadData( sound, data8, CHUNKSIZE, &read );
        for ( unsigned int i = 0; i < read; i++ ) {
          samples[ i ] = ( float ) data8[ i ] / 128;
        }
        bpmd.inputSamples( samples, read / channels );
      }
      cprogress++;
      if ( cprogress % 250 == 0 ) {
        /// @todo printing status (cprogress/totalsteps)
      }
    } while ( result == FMOD_OK && read == CHUNKSIZE );
    FMOD_Sound_Release(sound); sound = 0;

    float BPM = bpmd.getBpm();
    if ( BPM != 0. ) {
      BPM = Correct_BPM( BPM );
      if ( bpmsave ) Save_BPM( filename, BPM );
    }
    Print_BPM( BPM );
  }
}

float Str2BPM( QString str ) {
  QRegExp rx("[^\\.\\d]");
  str.replace(rx, "");
  if(!str.toFloat()) return 0;
  if(str.contains(".") >= 1) return str.toFloat();
  else {
    float BPM = str.toFloat();
    while( BPM > 250 ) BPM = BPM / 10;
    return BPM;
  }

  return 0;
}

QString BPM2str( float BPM ) {
  QString sbpm; sbpm.sprintf( "%.2f", BPM );
  sbpm = sbpm.rightJustify( 6, '0' );
  return sbpm;
}

QString Format_BPM( float BPM ) {
  QString sbpm;

  if( set_format == "0.00" ) {
    sbpm.sprintf( "%.2f", BPM );
  } else if( set_format == "0.0" ) {
    sbpm.sprintf( "%.1f", BPM );
  } else if( set_format == "0" ) {
    sbpm.sprintf( "%d", (int) BPM );
  } else if( set_format == "000.00" ) {
    sbpm.sprintf( "%.2f", BPM );
    sbpm = sbpm.rightJustify( 6, '0' );
  } else if( set_format == "000.0" ) {
    sbpm.sprintf( "%.1f", BPM );
    sbpm = sbpm.rightJustify( 5, '0' );
  } else if( set_format == "000" ) {
    sbpm.sprintf( "%d", (int) BPM );
    sbpm = sbpm.rightJustify( 3, '0' );
  } else if( set_format == "00000" ) {
    sbpm.sprintf( "%d", (int) (BPM * 100) );
    sbpm = sbpm.rightJustify( 5, '0' );
  } else {
    sbpm.sprintf( "%.2f", BPM );
  }

  return sbpm;
}

void Load_Settings() {
  qDebug("load settings");
  QSettings settings(QSettings::Ini);
  set_format = settings.readEntry("/BPMDetect/TBPMFormat", "0.00");
  set_skip = settings.readBoolEntry("/BPMDetect/SkipScanned", true);
  set_save = settings.readBoolEntry("/BPMDetect/SaveBPM", true);
}

void Save_Settings() {
  QSettings settings(QSettings::Ini);
  settings.writeEntry("/BPMDetect/TBPMFormat", set_format);
  settings.writeEntry("/BPMDetect/SkipScanned", set_skip);
  settings.writeEntry("/BPMDetect/SaveBPM", set_save);
  qDebug("settings saved");
}
