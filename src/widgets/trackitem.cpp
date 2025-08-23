#include <QtWidgets/QTreeWidget>

#include "track/track.h"
#include "trackitem.h"

TrackItem::TrackItem(QTreeWidget *parent, Track *track)
    : QTreeWidgetItem(parent,
                      {track->formatted(QStringLiteral("000.00")),
                       track->artist(),
                       track->title(),
                       track->formattedLength(),
                       QStringLiteral(""),
                       track->fileName()}),
      track_(track) {
}

TrackItem::~TrackItem() {
}

Track *TrackItem::track() const {
    return track_;
}

QProgressBar *TrackItem::progressBar() const {
    return progressBar_;
}

void TrackItem::setProgressBar(QProgressBar *bar) {
    progressBar_ = bar;
}
