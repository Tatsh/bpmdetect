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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qapplication.h>
#include <qprogressbar.h>
#include "qdroplistview.h"

#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"

#include "functions.h"
#include "images.h"

extern const char* description;
extern const char* version;

/**
 * @brief Constructor
 * @param parent parent widget
 * @param name name of widget
 * @param fl window flags
 */
dlgBPMDetect::dlgBPMDetect( QWidget* parent, const char* name, WFlags fl )
    : dlgBPMDetectdlg( parent, name, fl ) {
  stop = TRUE;
  citem = 0;

  setCaption("BPM Detect"); // TODO: append version

  Load_Settings();
//  chbSkipScanned->setChecked( !force );
//  chbSave->setChecked( bpmsave );
  chbSkipScanned->setChecked( set_skip );
  chbSave->setChecked( set_save );
  cbFormat->setCurrentText( set_format );
/*
#ifndef HAVE_TAGLIB
  chbSave->setChecked( false );
  chbSave->setEnabled( false );
#endif
*/

  /// Create TrackList menu
  ListMenu = new QPopupMenu( this );
  QLabel* caption = new QLabel(0);
  caption->setPixmap( menutop_xpm );
  caption->setScaledContents(true);
  ListMenu->insertItem( caption );
  ListMenu->insertItem( "Add files", this, SLOT( addFiles() ) );
  ListMenu->insertItem( "Add directory", this, SLOT( addDir() ) );
  ListMenu->insertSeparator();
  ListMenu->insertItem( "Remove selected tracks", this, SLOT( removeSelected() ) );
  ListMenu->insertItem( "Remove tracks with BPM", this, SLOT( clearDetected() ) );
  ListMenu->insertItem( "Clear track list", this, SLOT( clearTrackList() ) );
  ListMenu->insertSeparator();
  ListMenu->insertItem( "Test BPM", this, SLOT( testBPM() ) );

  TrackList->addColumn("BPM", 60);
  TrackList->addColumn("Artist", 200);
  TrackList->addColumn("Title", 200);
  TrackList->addColumn("Length", 60);
  TrackList->addColumn("Filename", 400);

  setCaption("BPM Detect");
  connect(TrackList, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(listMenuPopup( QListViewItem*, const QPoint& )));
  connect(TrackList, SIGNAL(keyPress(QKeyEvent *)),
    this, SLOT(trackListKeyPressed( QKeyEvent *)));
  connect(TrackList, SIGNAL(drop(QDropEvent *)),
    this, SLOT(dropped(QDropEvent *)));
}

/// @brief Destructor
dlgBPMDetect::~dlgBPMDetect() {
  /// Stop detection
  if ( !stop ) start();
  /// Sae settings
  set_skip = chbSkipScanned->isChecked();
  set_save = chbSave->isChecked();
  set_format = cbFormat->currentText();
  Save_Settings();
}

/**
 * @brief Convert miliseconds to time string
 * @param ms miliseconds
 * @return time string
 */
inline QString dlgBPMDetect::msec2time( uint ms ) {
  static SONGTIME sTime;
  QString timestr;
  sTime.msecs = ms / 10;
  sTime.secs = sTime.msecs / 100;
  sTime.msecs = sTime.msecs - ( sTime.secs * 100 );
  sTime.mins = sTime.secs / 60;
  sTime.secs = sTime.secs - ( sTime.mins * 60 );
  QString s;
  timestr.sprintf( "%d:", sTime.mins );
  s.sprintf( "%d.", sTime.secs );
  timestr.append( s.rightJustify( 3, '0' ) );
  s.sprintf( "%d", sTime.msecs );
  timestr.append( s.rightJustify( 2, '0' ) );
  return timestr;
}

void dlgBPMDetect::enableControls(bool e) {
  btnAddFiles->setEnabled( e );
  btnAddDir->setEnabled( e );
  btnRemoveSelected->setEnabled( e );
  btnClearList->setEnabled( e );
  TrackList->setEnabled( e );
  cbFormat->setEnabled( e );

  if(e) {
    btnStart->setText( "Start" );
    lblCurrentTrack->setText("");
    TrackList->setSelectionMode(QListView::Extended);
    TotalProgress->setProgress( 0 );
    CurrentProgress->setProgress( 0 );
  } else {
    btnStart->setText( "Stop" );
    TrackList->setSelectionMode(QListView::Single);
  }

  TrackList->clearSelection();
}

/**
 * @brief Start/Stop detection
 * Detect BPM of all tracks in TrackList
 */
void dlgBPMDetect::start() {
  if ( !stop ) {      // Stop scanning
    stop = TRUE;
    enableControls( true );
  } else {            // Start scanning
    stop = FALSE;
    enableControls( false);

    QListViewItemIterator it( TrackList );
    if ( TrackList->childCount() )
      TotalProgress->setTotalSteps( TrackList->childCount() );

    for ( int progress = 0; it.current(); ++it ) {
      FMOD_SOUND *sound;
      FMOD_RESULT result;
      progress++;

      cout << "Track " << progress << " of " << TrackList->childCount() << endl;
      TotalProgress->setProgress( progress );
      TrackList->ensureItemVisible( it.current() );
      TrackList->setSelected(it.current(), TRUE);

      QString cfile = it.current()->text( TrackList->columns() - 1 );
      cfile = cfile.right( cfile.length() - cfile.findRev( "/" ) - 1 );
      lblCurrentTrack->setText( cfile );
      cout << cfile << endl;

      if( chbSkipScanned->isChecked() &&
          it.current()->text( 0 ).toFloat() > 50. &&
          it.current()->text( 0 ).toFloat() < 250.) {
        Print_BPM(it.current()->text( 0 ).toFloat());
        continue;
      }
      result = FMOD_System_CreateStream( SoundSystem,
                 it.current()->text( 4 ).local8Bit(),
                 FMOD_OPENONLY, 0, &sound );
      if ( result != FMOD_OK ) {
        cerr << FMOD_ErrorString( result ) << " : " <<
        it.current()->text( 4 ).local8Bit() << endl;
        Print_BPM(0);
        continue;
      }

      {
#define CHUNKSIZE 4096
        int16_t data16[ CHUNKSIZE / 2 ];
        int8_t data8[ CHUNKSIZE ];
        SAMPLETYPE samples[ CHUNKSIZE / 2 ];
        unsigned int length = 0, read;
        int channels = 2, bits = 16;
        float frequency = 44100;
        result = FMOD_Sound_GetLength( sound, &length, FMOD_TIMEUNIT_PCMBYTES );
        FMOD_Sound_GetDefaults( sound, &frequency, 0, 0, 0 );
        FMOD_Sound_GetFormat ( sound, 0, 0, &channels, &bits );

        if ( bits != 16 && bits != 8 ) {
          cerr << bits << " bit samples are not supported!" << endl;
          Print_BPM( 0 );
          cout << endl;
          continue;
        }

        BPMDetect bpmd( channels, ( int ) frequency );
        CurrentProgress->setTotalSteps( length / CHUNKSIZE );
        int cprogress = 0;
        do {
          if ( cprogress % 20 == 0 )
            CurrentProgress->setProgress( cprogress );
          if( bits == 16 ) {
            result = FMOD_Sound_ReadData( sound, data16, CHUNKSIZE, &read );
            for ( uint i = 0; i < read / 2; i++ ) {
              samples[ i ] = ( float ) data16[ i ] / 32768;
            }
            bpmd.inputSamples( samples, read / ( 2 * channels ) );
          } else if ( bits == 8 ) {
            result = FMOD_Sound_ReadData( sound, data8, CHUNKSIZE, &read );
            for ( uint i = 0; i < read; i++ ) {
              samples[ i ] = ( float ) data8[ i ] / 128;
            }
            bpmd.inputSamples( samples, read / channels );
          }
          cprogress++;
          if ( cprogress % 25 == 0 )
            qApp->processEvents();
        } while ( result == FMOD_OK && read == CHUNKSIZE && !stop );
        FMOD_Sound_Release(sound); sound = 0;

        if ( !stop ) {
          float BPM = bpmd.getBpm();
          if ( BPM != 0. ) BPM = Correct_BPM( BPM );
          it.current()->setText( 0, BPM2str(BPM) );
          /// Save BPM
          if ( BPM != 0. && chbSave->isChecked() )
            Save_BPM( it.current()->text( TrackList->columns() - 1 ), BPM );
          Print_BPM( BPM );
        }
      }
      lblCurrentTrack->setText( "" );
      if ( stop ) break;
    }
    stop = TRUE;
    enableControls( true );
    cout << "Done" << endl << endl;
  }
}

/**
 * @brief Add files to TrackList
 *
 * @param files is list of file names to add
 */
void dlgBPMDetect::addFiles( QStringList &files ) {
  QStringList::Iterator it = files.begin();
  while ( it != files.end() ) {
    QString bpm, artist, title, length;
    FMOD_SOUND *sound;
    FMOD_TAG tag;
    FMOD_RESULT result;
    unsigned int len;

    result = FMOD_System_CreateStream( SoundSystem,
               (*it).local8Bit(), FMOD_OPENONLY, 0, &sound );
    if ( result != FMOD_OK ) {
      ++it;
      cerr << FMOD_ErrorString( result ) << " : " << ( *it ) << endl;
      continue;
    }
    FMOD_SOUND_TYPE type;
    int bits = 0;
    FMOD_Sound_GetFormat ( sound, &type, 0, 0, &bits );
    if( bits != 16 && bits != 8 ) {
      FMOD_Sound_Release(sound);
      ++it;
      continue;
    }

    FMOD_Sound_GetLength( sound, &len, FMOD_TIMEUNIT_MS );
    length = msec2time( len );

    if( type == FMOD_SOUND_TYPE_WAV) {
      FMOD_Sound_Release(sound); sound = 0;
      TAGINFO tinf = GetTagInfoWAV( *it );
      artist = tinf.Artist;
      title = tinf.Title;
      bpm = tinf.BPM;
    } else if( type == FMOD_SOUND_TYPE_MPEG) {
      FMOD_Sound_Release(sound); sound = 0;
      TAGINFO tinf = GetTagInfoMPEG( *it);
      artist = tinf.Artist;
      title = tinf.Title;
      bpm = tinf.BPM;
    } else {
      if ( FMOD_Sound_GetTag( sound, "TBPM", 0, &tag ) == FMOD_OK ) {
        QString s = ( char* ) tag.data;
        bpm = BPM2str(Str2BPM(s));
      } else bpm = "000.00";
      if ( FMOD_Sound_GetTag( sound, "ARTIST", -1, &tag ) == FMOD_OK )
        artist = ( char* ) tag.data;
      if ( FMOD_Sound_GetTag( sound, "TITLE", -1, &tag ) == FMOD_OK )
        title = ( char* ) tag.data;
      else
        title = (*it).right( (*it).length() - (*it).findRev("/") - 1 );
      FMOD_Sound_Release(sound); sound = 0;
    }

    new QListViewItem( TrackList, bpm, artist, title, length, *it );
    ++it;
    qApp->processEvents();
  }
}

/**
 * @brief Add files to TrackList
 * Open file dialog and add selected files to TrackList
 */
void dlgBPMDetect::addFiles() {
  QStringList files;
  files += QFileDialog::getOpenFileNames(
            "Audio files (*.wav *.mp3 *.ogg *.flac)",
            recentpath, this, "Add tracks", "Select tracks" );
  if(files.count() > 0)
    recentpath = files[0].left(files[0].findRev( "/" ));
  addFiles( files );
}

/**
 * @brief Add directory to TrackList
 * Open file dialog to select path and add files from
 * directory including subdirectories to TrackList
 */
void dlgBPMDetect::addDir() {
  QString path = QFileDialog::getExistingDirectory (
            recentpath, this, 0, "Add directory" );

  if ( path != QString::null ) {
    recentpath = path;
    QStringList list;
    list += filesFromDir( path );
    if ( list.count() == 0 ) return;

    if ( !path.endsWith( "/" ) )
      path.append( "/" );

    QStringList files;
    for ( uint i = 0; i < list.count(); i++ ) {
      QString filename = path + list[ i ];
      files.append( filename );
    }

    addFiles( files );
  }
}

/**
 * @brief Popup Tracklist menu
 * @param item TrackList item
 * @param p popup point
 */
void dlgBPMDetect::listMenuPopup( QListViewItem* item, const QPoint &p ) {
  ListMenu->popup( p );
  citem = item;
}

/**
 * @brief Get all wav, ogg, flac and mp3 files
 * from directory path including files from subdirectories
 * @param path path from which the files are added
 * @return QStringList of files with relative paths
 * to @param path
 */
QStringList dlgBPMDetect::filesFromDir( QString path ) {
  QDir d( path ), f( path ); QStringList files;

  if ( d.exists( path ) ) {
    d.setFilter( QDir::Dirs | QDir::Hidden | QDir::NoSymLinks );
    f.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    f.setNameFilter( "*.wav *.mp3 *.ogg *.flac" );

    QStringList dirs = d.entryList();
    files = f.entryList();

    if ( dirs.count() ) {
      for ( QStringList::Iterator it = dirs.begin() ; it != dirs.end() ; ++it ) {
        if ( ( *it ) != "." && ( *it ) != ".." ) {
          QStringList nfiles = filesFromDir( d.absPath() + "/" + ( *it ) );
          if ( nfiles.count() ) {
            for ( QStringList::Iterator nit = nfiles.begin();
                  nit != nfiles.end(); ++nit ) {
              files.append( *it + "/" + *nit );
            }
          }
        }
      }
    }
    return files;
  }

  return files;
}

/// @brief Remove selected tracks from TrackList
void dlgBPMDetect::removeSelected() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if ( it.current() != TrackList->firstChild() && it.current() ->isSelected() ) {
      TrackList->removeItem( it.current() ); it--;
    }
  }
  if ( TrackList->firstChild() && TrackList->firstChild() ->isSelected() )
    TrackList->removeItem( TrackList->firstChild() );
  
}

/// @brief Show dialog to test detected BPM of current track
void dlgBPMDetect::testBPM() {
  if ( citem != NULL && citem->text( 0 ).toFloat() != 0. ) {
    float bpm = citem->text( 0 ).toFloat();
    dlgTestBPM tbpmd( SoundSystem, citem->text( TrackList->columns() - 1 ), bpm, this );
    int ret;
    ret = tbpmd.exec();
    tbpmd.stop();
  }
}

/// @brief Show about dialog
void dlgBPMDetect::showAbout() {
  QString abouttext = " \
Version:    \t%1 \n \
Description:\t%2 \n \
Author:     \tMartin Sakmar \n \
e-mail:     \tmartin.sakmar@gmail.com \n \
License:    \tGNU General Public License \
";
  abouttext.replace("%1", version);
  abouttext.replace("%2", description);
  QMessageBox::about(this, "About BPM Detect", abouttext);
}


/// @brief Clear the TrackList
void dlgBPMDetect::clearTrackList() {
  TrackList->clear();
}

/// @brief Clear tracks with detected BPM
void dlgBPMDetect::clearDetected() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if ( it.current() != TrackList->firstChild() ) {
      float fBPM = it.current()->text(0).toFloat();
      if(fBPM >= 50.) {
        TrackList->removeItem( it.current() );
        it--;
      }
    }
  }
  if ( TrackList->firstChild() ) {
    float fBPM = TrackList->firstChild()->text(0).toFloat();
    if(fBPM >= 50.) TrackList->removeItem( TrackList->firstChild() );
  }
}

void dlgBPMDetect::formatChanged(const QString &f) {
  set_format = f;
}

void dlgBPMDetect::trackListKeyPressed(QKeyEvent *e) {
  if(e->key() == Qt::Key_Delete) removeSelected();
}

void dlgBPMDetect::dropped(QDropEvent* e) {
  e->accept(1);
  QStringList files;
  if ( QUriDrag::decodeLocalFiles( e, files ) )
    addFiles(files);
  return;
}

#include "dlgbpmdetect.moc"

