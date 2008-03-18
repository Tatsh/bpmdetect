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

#include <qevent.h>
#include <BPMDetect.h>
#include <iostream>

#include "dlgbpmdetectdlg.h"

class QPopupMenu;
class QListViewItem;

using namespace std;
using namespace soundtouch;

class dlgBPMDetect : public dlgBPMDetectdlg {
  Q_OBJECT

public:
  /// Constructor
  dlgBPMDetect( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  /// Destructor
  ~dlgBPMDetect();

public slots:
  void slotStartStop();

  void slotAddFiles( QStringList &files );
  void slotRemoveSelected();
  void slotTestBPM();
  void slotShowAbout();
  void slotClearTrackList();
  void slotClearDetected();

protected:
  /// List of files from directory (including files from subdirectories)
  QStringList filesFromDir( QString path );
  void enableControls( bool enable );

  void loadSettings();
  void saveSettings();

  void setStarted( bool started );
  bool getStarted() const;
  void setRecentPath( QString path );
  QString getRecentPath() const;

protected slots:
  void slotAddDir();
  void slotAddFiles();
  void slotListKeyPressed(QKeyEvent *e);
  void slotListMenuPopup( QListViewItem*, const QPoint& );
  void slotDropped(QDropEvent* e);
  void slotClearBPM();

private:
  QPopupMenu* m_pListMenu;
  QListViewItem* m_pCurItem;
  QString m_qRecentPath;
  bool m_bStarted;
};

#endif // _BPMDETECTWIDGET_H_
