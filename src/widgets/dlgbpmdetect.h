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
    /** Slot to start/stop BPM detection. */
    void slotStartStop();
    /** Slot to add files. */
    void slotAddFiles(QStringList &files);

protected:
    /** List of files from directory (including files from subdirectories). */
    QStringList filesFromDir(const QString &path);
    /** Enable or disable controls. */
    void enableControls(bool enable);

    /** Load user settings. */
    void loadSettings();
    /** Save user settings. */
    void saveSettings();

    /** Set if detection has started. */
    void setStarted(bool started);
    /** Get the started flag. */
    bool getStarted() const;
    /** Set the most recent path for opening files. */
    void setRecentPath(const QString &path);
    /** Get the recent path. */
    QString getRecentPath() const;

protected Q_SLOTS:
    /** Slot to add a directory. */
    void slotAddDir();
    /** Slot to add files. */
    void slotAddFiles();
    /** Slot to start detection. */
    void slotStart();
    /** Slot to stop detection. */
    void slotStop();
    /** Slot for context menu. */
    void slotListMenuPopup(const QPoint &);
    /** Slot for dropping a file. */
    void slotDropped(QDropEvent *e);
    /**
     * Start detection on next track in the list.
     * @param skipped true if previous track was skipped, so BPM won't be saved.
     */
    void slotDetectNext(bool skipped = false);
    /** Slot when timer is done. */
    void slotTimerDone();
    /** Slot when selected files are removed. */
    void slotRemoveSelected();
    /** Slot to clear the whole list. */
    void slotClearTrackList();
    /** Slot to clear items on the list that have BPMs. */
    void slotClearDetected();
    /** Slot to test the BPM. */
    void slotTestBPM();
    /** Slot to clear the BPM. */
    void slotClearBPM();
    /** Slot to save the BPM. */
    void slotSaveBPM();
    /** Slot to show the about dialog. */
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
