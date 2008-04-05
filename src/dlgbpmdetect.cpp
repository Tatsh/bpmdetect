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
#include <qspinbox.h>
#include "qdroplistview.h"

#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"

#include "images.h"

extern const char* version;

dlgBPMDetect::dlgBPMDetect( QWidget* parent, const char* name, WFlags fl )
    : dlgBPMDetectdlg( parent, name, fl ) {
  setStarted(false);
  m_pCurItem = 0;
  m_iCurTrackIdx = 0;

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
  m_pListMenu->insertSeparator();
  m_pListMenu->insertItem( "Save BPM", this, SLOT( slotSaveBPM() ) );
  m_pListMenu->insertItem( "Clear BPM", this, SLOT( slotClearBPM() ) );
  /// Add columns to TrackList
  TrackList->addColumn("BPM", 60);
  TrackList->addColumn("Artist", 200);
  TrackList->addColumn("Title", 200);
  TrackList->addColumn("Length", 60);
  TrackList->addColumn("Filename", 400);

  /// Connect signals with slots
  connect(TrackList, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListMenuPopup( QListViewItem*, const QPoint& )));
  connect(TrackList, SIGNAL(drop(QDropEvent *)),
    this, SLOT(slotDropped(QDropEvent *)));

  connect(btnStart, SIGNAL(clicked()), this, SLOT(slotStartStop()));

  m_pTrack = new Track("");

  connect(&m_qTimer, SIGNAL(timeout()), this, SLOT(slotTimerDone()));
  m_qTimer.start(20);
}

dlgBPMDetect::~dlgBPMDetect() {
  if ( getStarted() ) slotStop();
  saveSettings();
  delete m_pTrack;
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
  int minBPM = settings.readNumEntry("/BPMDetect/MinBPM", 80);
  int maxBPM = settings.readNumEntry("/BPMDetect/MaxBPM", 190);
  chbSkipScanned->setChecked( skip );
  chbSave->setChecked( save );
  cbFormat->setCurrentText( format );
  setRecentPath(recentpath);
  spMin->setValue(minBPM);
  spMax->setValue(maxBPM);
}

void dlgBPMDetect::saveSettings() {
  QSettings settings(QSettings::Ini);
  settings.writeEntry("/BPMDetect/TBPMFormat", cbFormat->currentText());
  settings.writeEntry("/BPMDetect/SkipScanned", chbSkipScanned->isChecked());
  settings.writeEntry("/BPMDetect/SaveBPM", chbSave->isChecked());
  settings.writeEntry("/BPMDetect/RecentPath", getRecentPath());
  settings.writeEntry("/BPMDetect/MinBPM", spMin->value());
  settings.writeEntry("/BPMDetect/MaxBPM", spMax->value());
#ifdef DEBUG
  qDebug("Settings saved");
#endif
}

/**
 ** Enable or disable controls
 ** @param enable true to enable, false to disable
 **/
void dlgBPMDetect::enableControls(bool enable) {
  btnAddFiles->setEnabled( enable );
  btnAddDir->setEnabled( enable );
  btnRemoveSelected->setEnabled( enable );
  btnClearList->setEnabled( enable );
  TrackList->setEnabled( enable );
  cbFormat->setEnabled( enable );
  spMin->setEnabled( enable );
  spMax->setEnabled( enable );

  if( enable ) {
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
  m_pCurItem = 0;
  m_iCurTrackIdx = 0;
}

void dlgBPMDetect::slotStartStop() {
  if ( getStarted() ) {
    slotStop();
  } else {
    slotStart();
  }
}

void dlgBPMDetect::slotStart() {
  if ( getStarted() || !TrackList->childCount() ) return;

  setStarted(true);
  enableControls(false);

  TotalProgress->setTotalSteps( TrackList->childCount() * 100 );
  TotalProgress->setProgress(0);
  m_pTrack->setMinBPM(spMin->value());
  m_pTrack->setMaxBPM(spMax->value());
  slotDetectNext();
}

void dlgBPMDetect::slotStop() {
  m_pTrack->stop();
  setStarted(false);
  lblCurrentTrack->setText( "" );
  enableControls(true);
}

void dlgBPMDetect::slotDetectNext(bool skipped) {
  if(!m_pCurItem) {
    m_pCurItem = TrackList->firstChild();
  } else {
    if(!skipped) {
      m_pCurItem->setText(0, m_pTrack->strBPM("000.00"));
      if ( chbSave->isChecked() )
        m_pTrack->saveBPM(cbFormat->currentText());
    }
    m_pCurItem = m_pCurItem->itemBelow();
  }

  if(!m_pCurItem) {
    slotStop();
    return;
  }

  TrackList->clearSelection();
  TrackList->ensureItemVisible(m_pCurItem);
  TrackList->setSelected(m_pCurItem, true);

  QString file = m_pCurItem->text( TrackList->columns() - 1 );
  lblCurrentTrack->setText( file.section('/', -1, -1) );
  double BPM = m_pCurItem->text(0).toDouble();
  if(chbSkipScanned->isChecked() && BPM > 0) {
    slotDetectNext(true);
    return;
  }

  TotalProgress->setProgress( 100 * m_iCurTrackIdx++ );

  m_pTrack->setFilename(file.local8Bit());
  m_pTrack->setRedetect(!chbSkipScanned->isChecked());
  m_pTrack->startDetection();
}

void dlgBPMDetect::slotTimerDone() {
  CurrentProgress->setProgress((int) (10 * m_pTrack->getProgress()));
  TotalProgress->setProgress(100*(m_iCurTrackIdx-1) + (int) m_pTrack->getProgress());
  if(getStarted() && m_pTrack->finished()) {
    slotDetectNext();
  }
}

void dlgBPMDetect::slotAddFiles( QStringList &files ) {
  QStringList::Iterator it = files.begin();
  while ( it != files.end() ) {
    Track track( (*it).local8Bit(), true );
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
  TrackList->slotRemoveSelected();
}

void dlgBPMDetect::slotTestBPM() {
  if ( m_pCurItem != 0 ) {
    float bpm = m_pCurItem->text( 0 ).toFloat();
    if(! bpm ) return;

    dlgTestBPM tbpmd( m_pCurItem->text( TrackList->columns() - 1 ), bpm, this );
    int ret;
    ret = tbpmd.exec();
  }
}

void dlgBPMDetect::slotShowAbout() {
  QString description = "Automatic BPM (Beat Per Minute) detecting application.";
  QString abouttext = " \
Version:    \t%1 \n \
Description:\t%2 \n \
License:    \tGNU General Public License \n\
 \n\
Author:     \tMartin Sakmar \n \
e-mail:     \tmartin.sakmar@gmail.com \n \
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

void dlgBPMDetect::slotDropped(QDropEvent* e) {
  e->accept(1);
  QStringList files;
  if ( QUriDrag::decodeLocalFiles( e, files ) )
    slotAddFiles(files);
  return;
}

void dlgBPMDetect::slotSaveBPM() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if(it.current()->isSelected()) {
      Track track(it.current()->text(TrackList->columns() - 1).local8Bit());
      track.setBPM(it.current()->text(0).toDouble());
      track.saveBPM();
    }
  }
}

void dlgBPMDetect::slotClearBPM() {
  int clear = -1;
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if(clear < 0) {
      clear = QMessageBox::warning(this, "Clear BPM",
                      "Do you want to clear BPMs?",
                      QMessageBox::Yes, QMessageBox::No, 0 );
      if(clear == QMessageBox::No) return;
    }
    if(it.current()->isSelected()) {
      Track track(it.current()->text(TrackList->columns() - 1).local8Bit());
      track.clearBPM();
      it.current()->setText(0, "000.00");
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

