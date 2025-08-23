// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtWidgets/QTreeWidgetItem>

class QProgressBar;
class QTreeWidget;
class Track;

class TrackItem : public QTreeWidgetItem {
public:
    TrackItem(QTreeWidget *parent, Track *track);
    ~TrackItem() override;
    Track *track() const;
    QProgressBar *progressBar() const;
    void setProgressBar(QProgressBar *bar);

private:
    Track *track_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
};
