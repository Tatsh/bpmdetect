// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <iostream>

#include <BPMDetect.h>
#include <QDropEvent>
#include <QMenu>
#include <QTimer>

#include "track/trackproxy.h"
#include "ui_dlgbpmdetect.h"

/** Main dialog of the application. */
class DlgBPMDetect : public QWidget, public Ui_DlgBPMDetect {
    Q_OBJECT
public:
    /** Constructor.
     * @param parent Parent widget.
     */
    DlgBPMDetect(QWidget *parent = nullptr);
    ~DlgBPMDetect() override;

public Q_SLOTS:
    /** Slot to add files. */
    void slotAddFiles(const QStringList &files);

protected Q_SLOTS:
    void slotStartStop();
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
    QString getRecentPath() const;
    QStringList filesFromDir(const QString &path) const;
    bool getStarted() const;
    void enableControls(bool enable);
    void loadSettings();
    void saveSettings();
    void setRecentPath(const QString &path);
    void setStarted(bool started);

    QProgressBar *m_pProgress;
    QMenu *m_pListMenu;
    QTreeWidgetItem *m_pCurItem;
    TrackProxy *m_pTrack;
    QTimer m_qTimer;
    QString m_qRecentPath;
    int m_iCurTrackIdx; // for total progress
    bool m_bStarted;
};
