#include <QtWidgets/QTreeWidget>

#include "track/track.h"
#include "trackitem.h"

TrackItem::TrackItem(QTreeWidget *parent, Track *track)
    : QTreeWidgetItem(parent,
                      {track->formatted(QStringLiteral("000.00")),
                       track->hasSavedBpm() ? QStringLiteral("✅") : QStringLiteral("❌"),
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

void TrackItem::refreshSavedBpmIndicator() {
    setText(1, track_->hasSavedBpm() ? QStringLiteral("✅") : QStringLiteral("❌"));
}

void TrackItem::setLastError(const QString &error) {
    setText(7, error);
}
