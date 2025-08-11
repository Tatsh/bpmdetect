// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <iostream>

#include <BPMDetect.h>
#include <QDropEvent>
#include <QMenu>
#include <QTimer>

#include "track/trackproxy.h"
#include "ui_dlgbpmdetect.h"

class DlgBPMDetect : public QWidget, public Ui_DlgBPMDetect {
    Q_OBJECT
public:
    DlgBPMDetect(QWidget *parent = nullptr);
    ~DlgBPMDetect() override;

public Q_SLOTS:
    void slotStartStop();
    void slotAddFiles(QStringList &files);

protected:
    /// List of files from directory (including files from subdirectories)
    QStringList filesFromDir(const QString &path);
    void enableControls(bool enable);

    void loadSettings();
    void saveSettings();

    void setStarted(bool started);
    bool getStarted() const;
    void setRecentPath(const QString &path);
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
    TrackProxy *m_pTrack;
    QTimer m_qTimer;
    QString m_qRecentPath;
    int m_iCurTrackIdx; // for total progress
    bool m_bStarted;
};
