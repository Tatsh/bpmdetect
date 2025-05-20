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

#pragma once

#include <QDropEvent>
#include <QMenu>
#include <QTimer>

#include <BPMDetect.h>
#include <iostream>

#include "trackproxy.h"
#include "ui_dlgbpmdetect.h"

class Q3PopupMenu;
class Q3ListViewItem;

using namespace std;
using namespace soundtouch;

class DlgBPMDetect : public QWidget, public Ui_DlgBPMDetect {
    Q_OBJECT
public:
    DlgBPMDetect(QWidget *parent = 0);
    ~DlgBPMDetect();

public Q_SLOTS:
    void slotStartStop();
    void slotAddFiles(QStringList &files);

protected:
    /// List of files from directory (including files from subdirectories)
    QStringList filesFromDir(QString path);
    void enableControls(bool enable);

    void loadSettings();
    void saveSettings();

    void setStarted(bool started);
    bool getStarted() const;
    void setRecentPath(QString path);
    QString getRecentPath() const;

protected Q_SLOTS:
    void slotAddDir();
    void slotAddFiles();

    void slotStart();
    void slotStop();

    void slotListMenuPopup(const QPoint &);
    void slotDropped(QDropEvent *e);

    void slotDetectNext(bool skipped = false);
    void slotTimerDone();

    void slotRemoveSelected();
    void slotClearTrackList();
    void slotClearDetected();
    void slotTestBPM();
    void slotClearBPM();
    void slotSaveBPM();
    void slotShowAbout();

private:
    QProgressBar *m_pProgress;
    QMenu *m_pListMenu;
    QTreeWidgetItem *m_pCurItem;
    int m_iCurTrackIdx; // for total progress
    QString m_qRecentPath;
    bool m_bStarted;
    TrackProxy *m_pTrack;
    QTimer m_qTimer;
};
