// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <BPMDetect.h>
#include <QtCore/QAtomicInt>
#include <QtCore/QSettings>

#include "ui_dlgbpmdetect.h"
#include "utils.h"

class AbstractBpmDetector;
class QDropEvent;
class QEventLoop;
class QMenu;
class Track;
class TrackItem;

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
    DlgBpmDetect(QWidget *parent = nullptr);
    ~DlgBpmDetect() override;
    /** Set the BPM detector. */
    void setDetector(AbstractBpmDetector *detector);

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
    /** Slot to show the About dialog. */
    void slotShowAbout();

private:
    QString recentPath() const;
    QStringList filesFromDir(const QString &path) const;
    void enableControls(bool enable);
    void loadSettings();
    void saveSettings();
    void setRecentPath(const QString &path);

    AbstractBpmDetector *detector_ = nullptr;
    QAtomicInt pendingTracks_ = 0;
    QEventLoop *innerEventLoop_ = nullptr;
    QMenu *columnMenu_ = nullptr;
    QMenu *listMenu_ = nullptr;
    QSettings settings_;
    QString recentPath_;
    QStringList displayedColumns_;
    bool editing_ = false;
    QModelIndex editingIndex_;
};
