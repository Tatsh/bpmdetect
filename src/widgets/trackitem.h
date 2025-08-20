#pragma once

#include <QtWidgets/QTreeWidgetItem>

class QTreeWidget;
class Track;

class TrackItem : public QTreeWidgetItem {
public:
    TrackItem(QTreeWidget *parent, Track *const track);
    ~TrackItem() override;
    Track *track() const;

private:
    Track *const track_ = nullptr;
};
