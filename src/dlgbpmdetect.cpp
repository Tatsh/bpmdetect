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
#include <qpopupmenu.h>
#include <qlistview.h>
#include <qsettings.h>
#include "qdroplistview.h"

#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"

#include "images.h"

#include "track.h"
// FIXME: do not use functions
#include "functions.h"

extern const char* version;


dlgBPMDetect::dlgBPMDetect( QWidget* parent, const char* name, WFlags fl )
    : dlgBPMDetectdlg( parent, name, fl ) {
  setStarted(false);
  m_pCurItem = 0;

  QString strcaption = "BPM Detect v";
  strcaption.append(version);
  setCaption(strcaption);

  loadSettings();

  /// Create TrackList menu
  m_pListMenu = new QPopupMenu( this );
  QLabel* caption = new QLabel(0);
  caption->setPixmap( menutop_xpm );
  caption->setScaledContents(true);
  m_pListMenu->insertItem( caption );
  m_pListMenu->insertItem( "Add files", this, SLOT( slotAddFiles() ) );
  m_pListMenu->insertItem( "Add directory", this, SLOT( slotAddDir() ) );
  m_pListMenu->insertSeparator();
  m_pListMenu->insertItem( "Remove selected tracks", this, SLOT( slotRemoveSelected() ) );
  m_pListMenu->insertItem( "Remove tracks with BPM", this, SLOT( slotClearDetected() ) );
  m_pListMenu->insertItem( "Clear list", this, SLOT( slotClearTrackList() ) );
  m_pListMenu->insertSeparator();
  m_pListMenu->insertItem( "Test BPM", this, SLOT( slotTestBPM() ) );
  m_pListMenu->insertItem( "Clear BPM", this, SLOT( slotClearBPM() ) );

  TrackList->addColumn("BPM", 60);
  TrackList->addColumn("Artist", 200);
  TrackList->addColumn("Title", 200);
  TrackList->addColumn("Length", 60);
  TrackList->addColumn("Filename", 400);

  connect(TrackList, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListMenuPopup( QListViewItem*, const QPoint& )));
  connect(TrackList, SIGNAL(keyPress(QKeyEvent *)),
    this, SLOT(slotListKeyPressed( QKeyEvent *)));
  connect(TrackList, SIGNAL(drop(QDropEvent *)),
    this, SLOT(slotDropped(QDropEvent *)));
}

dlgBPMDetect::~dlgBPMDetect() {
  if ( getStarted() ) slotStartStop();
  saveSettings();
}

void dlgBPMDetect::loadSettings() {
#ifdef DEBUG
  qDebug("Loading settings");
#endif
  QSettings settings(QSettings::Ini);
  QString format = settings.readEntry("/BPMDetect/TBPMFormat", "0.00");
  bool skip = settings.readBoolEntry("/BPMDetect/SkipScanned", true);
  bool save = settings.readBoolEntry("/BPMDetect/SaveBPM", true);
  QString recentpath = settings.readEntry("/BPMDetect/RecentPath", "");
  chbSkipScanned->setChecked( skip );
  chbSave->setChecked( save );
  cbFormat->setCurrentText( format );
  setRecentPath(recentpath);
}

void dlgBPMDetect::saveSettings() {
  QSettings settings(QSettings::Ini);
  settings.writeEntry("/BPMDetect/TBPMFormat", cbFormat->currentText());
  settings.writeEntry("/BPMDetect/SkipScanned", chbSkipScanned->isChecked());
  settings.writeEntry("/BPMDetect/SaveBPM", chbSave->isChecked());
  settings.writeEntry("/BPMDetect/RecentPath", getRecentPath());
#ifdef DEBUG
  qDebug("Settings saved");
#endif
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


// TODO: use Track instead of functions
void dlgBPMDetect::slotStartStop() {
  if ( getStarted() ) {      // Stop scanning
    setStarted(false);
    enableControls(true);
  } else {                   // Start scanning
    setStarted(true);
    enableControls(false);

    QListViewItemIterator it( TrackList );
    if ( TrackList->childCount() )
      TotalProgress->setTotalSteps( TrackList->childCount() );

    for ( int progress = 0; it.current(); ++it ) {
      FMOD_SOUND *sound;
      FMOD_RESULT result;
      progress++;

      TotalProgress->setProgress( progress );
      TrackList->ensureItemVisible( it.current() );
      TrackList->setSelected(it.current(), true);

      QString cfile = it.current()->text( TrackList->columns() - 1 );
      cfile = cfile.right( cfile.length() - cfile.findRev( "/" ) - 1 );
      lblCurrentTrack->setText( cfile );

      if( chbSkipScanned->isChecked() &&
          it.current()->text( 0 ).toFloat() > 50. &&
          it.current()->text( 0 ).toFloat() < 250.) {
        continue;
      }
      result = FMOD_System_CreateStream( SoundSystem,
                 it.current()->text( 4 ).local8Bit(),
                 FMOD_OPENONLY, 0, &sound );
      if ( result != FMOD_OK ) {
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
        } while ( result == FMOD_OK && read == CHUNKSIZE && getStarted() );
        FMOD_Sound_Release(sound); sound = 0;

        if ( getStarted() ) {
          float BPM = bpmd.getBpm();
          if ( BPM != 0. ) BPM = correctBPM( BPM );
          it.current()->setText( 0, bpm2str(BPM, "000.00") );
          /// Save BPM
          if ( BPM != 0. && chbSave->isChecked() )
            saveBPM( it.current()->text( TrackList->columns() - 1 ), BPM );
        }
      }
      lblCurrentTrack->setText( "" );
      if ( !getStarted() ) break;
    }
    setStarted(false);
    enableControls(true);
  }
}

void dlgBPMDetect::slotAddFiles( QStringList &files ) {
  QStringList::Iterator it = files.begin();
  while ( it != files.end() ) {
    Track track( *it, true );
    QString bpm, artist, title, length;

    bpm = track.strBPM("000.00");
    artist = track.getArtist();
    title = track.getTitle();
    length = track.strLength();
    new QListViewItem( TrackList, bpm, artist, title, length, *it );
    ++it;
    qApp->processEvents();
  }
}

void dlgBPMDetect::slotAddFiles() {
  QStringList files;
  files += QFileDialog::getOpenFileNames(
            "Audio files (*.wav *.mp3 *.ogg *.flac)",
            getRecentPath(), this, "Add tracks", "Select tracks" );
  if(files.count() > 0)
    setRecentPath(files[0].left(files[0].findRev( "/" )));
  slotAddFiles( files );
}

void dlgBPMDetect::slotAddDir() {
  QString path = QFileDialog::getExistingDirectory (
            getRecentPath(), this, 0, "Add directory" );

  if ( path != QString::null ) {
    setRecentPath(path);
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

    slotAddFiles( files );
  }
}

void dlgBPMDetect::slotListMenuPopup( QListViewItem* item, const QPoint &p ) {
  m_pListMenu->popup( p );
  m_pCurItem = item;
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

void dlgBPMDetect::slotRemoveSelected() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if ( it.current() != TrackList->firstChild() && it.current() ->isSelected() ) {
      TrackList->removeItem( it.current() ); it--;
    }
  }
  if ( TrackList->firstChild() && TrackList->firstChild() ->isSelected() )
    TrackList->removeItem( TrackList->firstChild() );
  
}

void dlgBPMDetect::slotTestBPM() {
  if ( m_pCurItem != 0 ) {
    float bpm = m_pCurItem->text( 0 ).toFloat();
    // TODO:
    if(! bpm ) return;

    dlgTestBPM tbpmd( SoundSystem, m_pCurItem->text( TrackList->columns() - 1 ), bpm, this );
    int ret;
    ret = tbpmd.exec();
    tbpmd.stop();
  }
}

void dlgBPMDetect::slotShowAbout() {
  QString description = "Automatic BPM (Beat Per Minute) detecting application. \n";
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


void dlgBPMDetect::slotClearTrackList() {
  TrackList->clear();
}

void dlgBPMDetect::slotClearDetected() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if ( it.current() != TrackList->firstChild() ) {
      float fBPM = it.current()->text(0).toFloat();
      if(fBPM > 0) {
        TrackList->removeItem( it.current() );
        it--;
      }
    }
  }
  if ( TrackList->firstChild() ) {
    float fBPM = TrackList->firstChild()->text(0).toFloat();
    if(fBPM > 0) TrackList->removeItem( TrackList->firstChild() );
  }
}

void dlgBPMDetect::slotListKeyPressed(QKeyEvent *e) {
  if(e->key() == Qt::Key_Delete) slotRemoveSelected();
}

void dlgBPMDetect::slotDropped(QDropEvent* e) {
  e->accept(1);
  QStringList files;
  if ( QUriDrag::decodeLocalFiles( e, files ) )
    slotAddFiles(files);
  return;
}

void dlgBPMDetect::slotClearBPM() {
  // TODO: message box
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if(it.current()->isSelected()) {
      Track track(it.current()->text(TrackList->columns() - 1));
      track.clearBPM();
    }
  }
}

void dlgBPMDetect::setStarted(bool started) {
  m_bStarted = started;
}

bool dlgBPMDetect::getStarted() const {
  return m_bStarted;
}

void dlgBPMDetect::setRecentPath(QString path) {
  m_qRecentPath = path;
}

QString dlgBPMDetect::getRecentPath() const {
  return m_qRecentPath;
}

#include "dlgbpmdetect.moc"

