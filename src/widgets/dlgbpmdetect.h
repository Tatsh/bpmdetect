// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <BPMDetect.h>
#include <QtCore/QTimer>

#include "track/trackproxy.h"
#include "ui_dlgbpmdetect.h"

class QDropEvent;
class QMenu;

/** Main dialog of the application. */
class DlgBpmDetect : public QWidget, public Ui_DlgBpmDetect {
    Q_OBJECT
#ifdef TESTING
    friend class DlgBpmDetectTest;
#endif
public:
    /** Constructor.
     * @param parent Parent widget.
     */
    explicit DlgBpmDetect(QWidget *parent = nullptr);
    ~DlgBpmDetect() override;

public Q_SLOTS:
    /** Slot to add files. */
    void slotAddFiles(const QStringList &files);

protected Q_SLOTS:
    /** Slot to start or stop BPM detection. */
    void slotStartStop();
    /** Slot to add a directory of files. */
    void slotAddDir();
    /** Slot to add files via file dialog. */
    void slotAddFiles();
    /** Slot to start BPM detection. */
    void slotStart();
    /** Slot to stop BPM detection. */
    void slotStop();
    /**
     * Slot to show context menu for the track list.
     * @param point Position for the menu popup.
     */
    void slotListMenuPopup(const QPoint &point);
    /**
     * Slot to handle files dropped onto the widget.
     * @param e Drop event containing dropped files.
     */
    void slotDropped(QDropEvent *e);
    /**
     * Slot to detect BPM for the next track.
     * @param skipped Whether the current track was skipped.
    */
    void slotDetectNext(bool skipped = false);
    /** Slot called when the timer finishes. */
    void slotTimerDone();
    /** Slot to remove selected tracks from the list. */
    void slotRemoveSelected();
    /** Slot to clear the track list. */
    void slotClearTrackList();
    /** Slot to clear detected BPM values. */
    void slotClearDetected();
    /** Slot to test BPM detection. */
    void slotTestBpm();
    /** Slot to clear BPM results. */
    void slotClearBpm();
    /** Slot to save detected BPM values. */
    void slotSaveBpm();
    /** * Slot to show the About dialog. */
    void slotShowAbout();

private:
    QString recentPath() const;
    QStringList filesFromDir(const QString &path) const;
    bool started() const;
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
