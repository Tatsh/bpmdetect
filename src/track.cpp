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

#ifndef NO_GUI
# include <qapplication.h>
#endif

#include "BPMDetect.h"

#include <fmodex/fmod_errors.h>
#include <stdlib.h>

using namespace std;
using namespace soundtouch;

extern FMOD_SYSTEM* SoundSystem;

static double _dMinBPM = 80.;
static double _dMaxBPM = 185.;

Track::Track( string filename, bool readtags ) {
  init();
  m_sound = 0;
  m_system = SoundSystem;
  m_iCurPosBytes = 0;
#ifndef NO_GUI
  m_iPriority = QThread::IdlePriority;
#endif
  setFilename( filename, readtags );
}

Track::Track( const char* filename, bool readtags ) {
  init();
  m_sound = 0;
  m_system = SoundSystem;
  m_iCurPosBytes = 0;
#ifndef NO_GUI
  m_iPriority = QThread::IdlePriority;
#endif
  setFilename( filename, readtags );
}

Track::~Track() {
  close();
}

void Track::init() {
  setTrackType(TYPE_UNKNOWN);
  setValid(false);
  setSamplerate(0);
  setSampleBytes(0);
  setChannels(0);
  setBPM(0);
  setProgress(0);
  setLength(0);
  setStartPos(0);
  setEndPos(0);
}

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
  while( BPM > 300 ) BPM = BPM / 10;
  return BPM;
}

string Track::bpm2str( double dBPM, string format ) {
  #define BPM_LEN 10
  char buffer[BPM_LEN];

  if( format == "0.0" ) {
    snprintf(buffer, BPM_LEN, "%.1f", dBPM );
  } else if( format == "0" ) {
    snprintf(buffer, BPM_LEN, "%d", (int) dBPM );
  } else if( format == "000.00" ) {
    snprintf(buffer, BPM_LEN, "%06.2f", dBPM );
  } else if( format == "000.0" ) {
    snprintf(buffer, BPM_LEN, "%05.1f", dBPM );
  } else if( format == "000" ) {
    snprintf(buffer, BPM_LEN, "%03d", (int) dBPM );
  } else if( format == "00000" ) {
    snprintf(buffer, BPM_LEN, "%05d", (int) dBPM * 100. );
  } else { // all other formats are converted to "0.00"
    snprintf(buffer, BPM_LEN, "%.2f", dBPM );
  }

  string sBPM = buffer;
  return sBPM;
}

string Track::strBPM( std::string format) {
  return bpm2str(getBPM(), format);
}

void Track::setFilename( const char* filename, bool readtags ) {
  string strfname = filename;
  setFilename(strfname, readtags);
}

void Track::setFilename( string filename, bool readtags ) {
#ifndef NO_GUI
  // only when the thread is not running
  if(running()) {
  #ifdef DEBUG
    qDebug("setFilename: thread not finished");
  #endif
    return;
  }
#endif

  close();
  init();
  m_sFilename = filename;
  open();
  if(readtags) readTags();
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

void Track::setArtist( string artist ) {
  m_sArtist = artist;
}

string Track::getArtist() const {
  return m_sArtist;
}

void Track::setTitle( string title ) {
  m_sTitle = title;
}

string Track::getTitle() const {
  return m_sTitle;
}

void Track::setRedetect(bool redetect) {
  m_bRedetect = redetect;
}

bool Track::getRedetect() const {
  return m_bRedetect;
}

void Track::setStartPos( uint ms ) {
  if(ms > getLength()) return;
  m_iStartPos = ms;
  if(m_iEndPos < m_iStartPos) {
    uint tmp = m_iEndPos;
    m_iEndPos = m_iStartPos;
    m_iStartPos = tmp;
  }
}

uint Track::getStartPos() const {
  return m_iStartPos;
}

void Track::setEndPos( uint ms ) {
  if(ms > getLength()) return;
  m_iEndPos = ms;
  if(m_iEndPos < m_iStartPos) {
    uint tmp = m_iEndPos;
    m_iEndPos = m_iStartPos;
    m_iStartPos = tmp;
  }
}

uint Track::getEndPos() const {
  return m_iEndPos;
}

double Track::getProgress() const {
  return m_dProgress;
}

void Track::setProgress(double progress) {
  if( 0 <= progress && progress <= 100)
    m_dProgress = progress;
}

void Track::setTrackType( TRACKTYPE type ) {
  m_eType = type;
}

TRACKTYPE Track::getTrackType() const {
  return m_eType;
}

void Track::setSamplerate( int samplerate ) {
  m_iSamplerate = samplerate;
}

int Track::getSamplerate() const {
  return m_iSamplerate;
}

void Track::setSampleBytes( int bytes ) {
  if(bytes < 0) return;
  if(bytes > 4) {
    if(!(bytes % 8)) {
      bytes = bytes / 8;
    } else {
    #ifdef DEBUG
      cerr << "Error: setSampleBytes: " << bytes << endl;
    #endif
      return;
    }
  }
  if(bytes > 4) return;
  m_iSampleBytes = bytes;
}

int Track::getSampleBits() const {
  return 8 * getSampleBytes();
}

int Track::getSampleBytes() const {
  return m_iSampleBytes;
}

void Track::setChannels( int channels) {
  m_iChannels = channels;
}

int Track::getChannels() const {
  return m_iChannels;
}

void Track::open() {
  if(isValid()) close();
  FMOD_RESULT result;
  m_iCurPosBytes = 0;
  string filename = getFilename();
  result = FMOD_System_CreateStream( m_system,
               filename.c_str(), FMOD_OPENONLY, 0, &m_sound );
  if ( result != FMOD_OK ) {
    init();
    m_sound = 0;
    return;
  }

  FMOD_SOUND_TYPE type;
  TRACKTYPE ttype = TYPE_UNKNOWN;
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
  setTrackType(ttype);
}

void Track::close() {
  if(running()) {
  #ifdef DEBUG
    clog << "close: detection not finished, stopping..." << endl;
  #endif
    stop();
    wait();
  }

  if(m_sound) FMOD_Sound_Release(m_sound);
  m_sound = 0;
  m_iCurPosBytes = 0;
  init();
}

void Track::seek( uint ms ) {
  if(isValid()) {
    // TODO: seek
    uint pos = (ms * getSamplerate() * getSampleBytes()) / 1000;
    FMOD_RESULT res = FMOD_Sound_SeekData(m_sound, pos);
    if(res = FMOD_OK) {
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

uint Track::currentPos() {
  if(isValid()) {
    unsigned long long pos = 1000*m_iCurPosBytes / (getSamplerate()*getChannels()*getSampleBytes());
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
int Track::readSamples( SAMPLETYPE* buffer, int num ) {
  if(!isValid()) return -1;

  FMOD_RESULT result;
  uint readbytes = 0;
  int sbytes = getSampleBytes();
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
void Track::saveBPM( string format ) {
  string filename = getFilename();
  string sBPM = bpm2str( getBPM(), format );
  TRACKTYPE type = getTrackType();

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

#ifndef NO_GUI
void Track::startDetection() {
#ifdef DEBUG
  if(running()) {
    qDebug("Start: thread is running");
    return;
  }
#endif
  start(m_iPriority);
}
#endif

/**
 * @brief Detect BPM of one track
 * @return detected BPM
 */
double Track::detectBPM() {
  if(!isValid()) {
  #ifdef DEBUG
    cerr << "detectBPM: track not valid" << endl;
  #endif
    return 0;
  }

  setProgress(0);
  m_bStop = false;

  double oldbpm = getBPM();
  if ( !getRedetect() && oldbpm != 0 ) {
    return oldbpm;
  }

//#define NUMSAMPLES 16384
#define NUMSAMPLES 32768
  int channels = getChannels();
  int samplerate = getSamplerate();
  SAMPLETYPE* samples = new SAMPLETYPE[NUMSAMPLES];

  uint totalsteps = getEndPos() - getStartPos();
  BPMDetect bpmd( channels, samplerate );

  uint cprogress = 0, pprogress = 0;
  int readsamples = 0;
  while(!m_bStop && 0 < (readsamples = readSamples(samples, NUMSAMPLES))) {
    bpmd.inputSamples( samples, readsamples/channels );
    cprogress = currentPos() - getStartPos();

    setProgress(100.*cprogress / (double) totalsteps );
    #ifdef NO_GUI
    while ( (100*cprogress/totalsteps) > pprogress ) {
      ++pprogress;
      clog << "\r" << (100*cprogress/totalsteps) << "% " << flush;
    }
    #endif
  }

  delete [] samples;
#ifdef NO_GUI
  clog << "\r" << flush;
#endif
  setProgress(100);
  if(m_bStop) return 0;
  double BPM = bpmd.getBpm();
  BPM = correctBPM(BPM);
  setBPM(BPM);
  return BPM;
}

void Track::readTags() {
  string filename = getFilename();
  TRACKTYPE type = getTrackType();

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

void Track::readTagsMPEG() {
  string filename = getFilename();
//#ifdef HAVE_ID3LIB
  ID3_Tag tag( filename.c_str() );
  if(char* sArtist = ID3_GetArtist(&tag)) {
    setArtist(sArtist);
  }
  if(char* sTitle = ID3_GetTitle(&tag)) {
    setTitle(sTitle);
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
//#elif defined(HAVE_TAGLIB)
  TagLib::MPEG::File f(filename, false);

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
  if(getTitle().empty())
    setTitle(filename.substr(filename.find_last_of("/") + 1));
  setBPM(str2bpm(sbpm));
}

void Track::readTagsWAV() {
  string filename = getFilename();

#ifdef HAVE_TAGLIB
  TagLib::MPEG::File f(filename.c_str(), false);
  long pos = f.rfind("ID3", TagLib::File::End);
  if(pos < 0) pos = f.length();

  TagLib::ID3v2::Tag tag(&f, pos);
  setArtist(tag.artist().toCString());
  setTitle(tag.title().toCString());

  TagLib::List<TagLib::ID3v2::Frame*> lst = tag.frameList("TBPM");
  string sbpm = "000.00";
  if(lst.size() > 0) {
    TagLib::ID3v2::Frame* frame = lst[0];
    sbpm = frame->toString().toCString();
  }
#endif
  // set filename (without path) as title if the title is empty
  if(getTitle().empty())
    setTitle(filename.substr(filename.find_last_of("/") + 1));
  setBPM(str2bpm(sbpm));
}

void Track::readTagsOGG() {
  string filename = getFilename();
// TODO: read OGG tags
  setTitle(filename.substr(filename.find_last_of("/") + 1));
}

void Track::readTagsFLAC() {
  string filename = getFilename();
// TODO: read FLAC tags
  setTitle(filename.substr(filename.find_last_of("/") + 1));
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
  //f.save();
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

void Track::clearBPMMPEG() {
#ifdef HAVE_TAGLIB
  string filename = getFilename();
  TagLib::MPEG::File f( filename.c_str(), false );
  TagLib::ID3v2::Tag* tag = f.ID3v2Tag(true);
  if(tag == NULL) {
    return;
  }
  tag->removeFrames("TBPM");
  f.save();
#endif
}

void Track::clearBPMWAV() {
#ifdef HAVE_TAGLIB
  string filename = getFilename();
  TagLib::MPEG::File f( filename.c_str(), false );
  long offset = f.rfind("ID3", TagLib::File::End);
  if(offset < 0) offset = f.length();           // ID3 tag offset
  TagLib::ID3v2::Tag tag(&f, offset);
  tag.removeFrames("TBPM");
  TagLib::ByteVector tdata = tag.render();
  f.seek(offset);
  f.writeBlock(tdata);
  //f.save();
#endif
}

void Track::clearBPMOGG() {
#ifdef HAVE_TAGLIB
  string filename = getFilename();
  TagLib::Ogg::Vorbis::File f( filename.c_str(), false );
  TagLib::Ogg::XiphComment* tag = f.tag();
  if(tag == NULL) {
    return;
  }
  tag->removeField("TBPM");
  f.save();
#endif
}

void Track::clearBPMFLAC() {
#ifdef HAVE_TAGLIB
  string filename = getFilename();
  TagLib::FLAC::File f( filename.c_str(), false );
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

void Track::clearBPM() {
  TRACKTYPE type = getTrackType();

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

unsigned int Track::getLength() const {
  return m_iLength;
}

void Track::setLength( unsigned int msec ) {
  m_iLength = msec;
}

void Track::stop() {
  m_bStop = true;
}

string Track::strLength() {
  uint length = getLength();

  uint csecs = length / 10;
  uint secs = csecs / 100;
  csecs = csecs % 100;
  uint mins = secs / 60;
  secs = secs % 60;

#define TIME_LEN 20
  char buffer[TIME_LEN];
  snprintf(buffer, TIME_LEN, "%d:%02d.%02d", mins, secs, csecs);
  string timestr = buffer;
  return timestr;
}

#ifndef NO_GUI
void Track::run() {
  detectBPM();
  setProgress(0);
}

void Track::setPriority(QThread::Priority priority) {
  m_iPriority = priority;
}
#endif
