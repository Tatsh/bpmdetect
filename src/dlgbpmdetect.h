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

#ifndef _BPMDETECTWIDGET_H_
#define _BPMDETECTWIDGET_H_

#include <qpopupmenu.h>
#include <qlistview.h>
#include <qevent.h>

#include <fmodex/fmod.hpp>
#include <fmodex/fmod_errors.h>
#include <BPMDetect.h>

#include <iostream>

#include "dlgbpmdetectdlg.h"
#include "functions.h"

using namespace std;
using namespace soundtouch;

struct SONGTIME {
  uint msecs;  ///< miliseconds
  uint secs;   ///< seconds
  uint mins;   ///< minutes
};

class dlgBPMDetect : public dlgBPMDetectdlg {
  Q_OBJECT

public:
  /// Constructor
  dlgBPMDetect( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  /// Destructor
  ~dlgBPMDetect();

public slots:
  /// Start/Stop BPM detection
  void start();
  /// Add files from directory including subdirectories
  void addDir();
  /// Add files to TrackList (QFileDialog)
  void addFiles();
  /// Add files to TrackList from QStringList
  void addFiles( QStringList &files );
  /// Popup list menu
  void listMenuPopup( QListViewItem*, const QPoint& );
  /// Remove selected tracks from TrackList
  void removeSelected();
  /// Open dialog for BPM testing
  void testBPM();
  /// Show about dialog
  void showAbout();
  /// Remove all tracks from TrackList
  void clearTrackList();
  /// Remove tracks with detected BPM
  void clearDetected();

protected:
  /// Return list of files from directory (including subdirectories)
  QStringList filesFromDir( QString path );
  /// Convert miliseconds to time string
  QString msec2time( uint ms );
  void enableControls(bool enable);

protected slots:
  void formatChanged(const QString &f);
  void trackListKeyPressed(QKeyEvent *e);
  void dropped(QDropEvent* e);

private:
  QPopupMenu* ListMenu; ///< TrackList popup menu
  QListViewItem* citem; ///< Current TrackList item
  QString recentpath;   ///< Recent QFileDialog path
  bool stop;            ///< start or stop BPM detection
};

#endif // _BPMDETECTWIDGET_H_
