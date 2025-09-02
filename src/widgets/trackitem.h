// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtWidgets/QTreeWidgetItem>

class QProgressBar;
class QTreeWidget;
class Track;

class TrackItem : public QTreeWidgetItem {
public:
    /** Constructor. */
    TrackItem(QTreeWidget *parent, Track *track);
    ~TrackItem() override;
    /** Get the track. */
    Track *track() const;
    /** Get the progress bar. */
    QProgressBar *progressBar() const;
    /** Refresh the saved BPM indicator. */
    void refreshSavedBpmIndicator();
    /** Set the last error message. */
    void setLastError(const QString &error);
    /** Set the progress bar. */
    void setProgressBar(QProgressBar *bar);

private:
    Track *track_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
};
