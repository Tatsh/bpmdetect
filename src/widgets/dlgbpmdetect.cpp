// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QMimeData>
#include <QtCore/QString>
#include <QtGui/QCursor>
#include <QtGui/QDropEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStyledItemDelegate>

#include "debug.h"
#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"
#include "ffmpegutils.h"
#include "qdroplistview.h"
#include "track/track.h"
#include "trackitem.h"
#include "trackitemdelegate.h"

#define kProgressColumn 5

const auto headerLabels = QStringList{QObject::tr("BPM", "BPM header label"),
                                      QObject::tr("Saved", "Saved header label"),
                                      QObject::tr("Artist", "Artist header label"),
                                      QObject::tr("Title", "Title header label"),
                                      QObject::tr("Length", "Length header label"),
                                      QObject::tr("Progress", "Progress header label"),
                                      QObject::tr("Filename", "Filename header label"),
                                      QObject::tr("Last Error", "Last Error header label")};

DlgBpmDetect::DlgBpmDetect(QWidget *parent) : QWidget(parent), columnMenu_(new QMenu(this)) {
    setupUi(this);

    loadSettings();

    // Create TrackList menu
    listMenu_ = new QMenu(TrackList);
    listMenu_->addAction(tr("Add files"), [this]() { slotAddFiles(); });
    listMenu_->addAction(tr("Add directory"), this, &DlgBpmDetect::slotAddDir);
    listMenu_->addSeparator();
    listMenu_->addAction(tr("Remove selected tracks"), this, &DlgBpmDetect::slotRemoveSelected);
    listMenu_->addAction(tr("Remove tracks with BPM"), this, &DlgBpmDetect::slotClearDetected);
    listMenu_->addAction(tr("Clear list"), this, &DlgBpmDetect::slotClearTrackList);
    listMenu_->addSeparator();
    listMenu_->addAction(tr("Test BPM"), this, &DlgBpmDetect::slotTestBpm);
    listMenu_->addSeparator();
    listMenu_->addAction(tr("Save BPM"), this, &DlgBpmDetect::slotSaveBpm);
    listMenu_->addAction(tr("Clear BPM"), this, &DlgBpmDetect::slotClearBpm);
    TrackList->setHeaderLabels(headerLabels);
    TrackList->setColumnWidth(0, 60);
    TrackList->setColumnWidth(1, 25);
    TrackList->setColumnWidth(2, 200);
    TrackList->setColumnWidth(3, 200);
    TrackList->setColumnWidth(4, 60);
    TrackList->setColumnWidth(5, 100);
    TrackList->setColumnWidth(6, 400);
    TrackList->setColumnWidth(7, 200);
    connect(TrackList,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(slotListMenuPopup(const QPoint &)));
    connect(TrackList, &QDropListView::drop, this, &DlgBpmDetect::slotDropped);
    TrackList->header()->restoreState(
        settings_.value(QStringLiteral("/BPMDetect/HeaderState")).toByteArray());
    TrackList->header()->restoreGeometry(
        settings_.value(QStringLiteral("/BPMDetect/HeaderGeometry")).toByteArray());
    const auto addColumnMenuAction = [this](const QString &name, int column) {
        auto action = columnMenu_->addAction(name, [this, column]() {
            auto isNewlyHidden = !TrackList->isColumnHidden(column);
            TrackList->setColumnHidden(column, isNewlyHidden);
            columnMenu_->actions()[column - 1]->setChecked(!isNewlyHidden);
        });
        action->setCheckable(true);
        action->setChecked(!TrackList->isColumnHidden(column));
    };
    for (auto i = 1; i < headerLabels.size(); ++i) { // Skip BPM.
        addColumnMenuAction(headerLabels[i], i);
    }
    TrackList->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    TrackList->header()->setSectionsMovable(false);
    connect(TrackList->header(), &QDropListView::customContextMenuRequested, [this](QPoint pos) {
        columnMenu_->popup(TrackList->header()->mapToGlobal(pos));
    });
    TrackList->setItemDelegate(new TrackItemDelegate(this));
    connect(TrackList, &QDropListView::itemChanged, [this](QTreeWidgetItem *item, int column) {
        if (column != 0) {
            return;
        }
        auto trackItem = static_cast<TrackItem *>(item);
        trackItem->resetLastError();
        trackItem->setText(1, QStringLiteral(""));
        trackItem->track()->setBpm(Track::correctBpm(item->text(0).toDouble()));
        trackItem->setText(0, trackItem->track()->formatted());
        if (chbSave->isChecked() && trackItem->track()->hasValidBpm()) {
            trackItem->track()->saveBpm();
            trackItem->refreshSavedBpmIndicator();
        }
    });

    connect(btnStart, &QPushButton::clicked, this, &DlgBpmDetect::slotStartStop);
}

DlgBpmDetect::~DlgBpmDetect() {
    if (innerEventLoop_ && innerEventLoop_->isRunning()) {
        // LCOV_EXCL_START
        slotStop();
        // LCOV_EXCL_STOP
    }
    saveSettings();
}

void DlgBpmDetect::loadSettings() {
    QString format =
        settings_.value(QStringLiteral("/BPMDetect/TBPMFormat"), QStringLiteral("0.00")).toString();
    auto skip = settings_.value(QStringLiteral("/BPMDetect/SkipScanned"), true).toBool();
    auto save = settings_.value(QStringLiteral("/BPMDetect/SaveBPM"), false).toBool();
    auto recentPath =
        settings_.value(QStringLiteral("/BPMDetect/RecentPath"), QStringLiteral("")).toString();
    auto minBPM = settings_.value(QStringLiteral("/BPMDetect/MinBPM"), 80).toInt();
    auto maxBPM = settings_.value(QStringLiteral("/BPMDetect/MaxBPM"), 190).toInt();
    chbSkipScanned->setChecked(skip);
    chbSave->setChecked(save);
    auto idx = cbFormat->findText(format);
    if (idx >= 0) {
        cbFormat->setCurrentIndex(idx);
    }
    setRecentPath(recentPath);
    spMin->setValue(minBPM);
    spMax->setValue(maxBPM);
    restoreGeometry(
        settings_.value(QStringLiteral("/BPMDetect/Geometry"), saveGeometry()).toByteArray());
    move(settings_.value(QStringLiteral("/BPMDetect/Position"), pos()).toPoint());
    resize(settings_.value(QStringLiteral("/BPMDetect/Size"), size()).toSize());
}

void DlgBpmDetect::saveSettings() {
    QSettings settings;
    settings.setValue(QStringLiteral("/BPMDetect/TBPMFormat"), cbFormat->currentText());
    settings.setValue(QStringLiteral("/BPMDetect/SkipScanned"), chbSkipScanned->isChecked());
    settings.setValue(QStringLiteral("/BPMDetect/SaveBPM"), chbSave->isChecked());
    settings.setValue(QStringLiteral("/BPMDetect/RecentPath"), recentPath());
    settings.setValue(QStringLiteral("/BPMDetect/MinBPM"), spMin->value());
    settings.setValue(QStringLiteral("/BPMDetect/MaxBPM"), spMax->value());
    settings.setValue(QStringLiteral("/BPMDetect/Geometry"), saveGeometry());
    settings.setValue(QStringLiteral("/BPMDetect/Position"), pos());
    settings.setValue(QStringLiteral("/BPMDetect/Size"), size());
    settings.setValue(QStringLiteral("/BPMDetect/HeaderState"), TrackList->header()->saveState());
    settings.setValue(QStringLiteral("/BPMDetect/HeaderGeometry"),
                      TrackList->header()->saveGeometry());
    settings.sync();
}

void DlgBpmDetect::enableControls(bool enable) {
    btnAddFiles->setEnabled(enable);
    btnAddDir->setEnabled(enable);
    btnRemoveSelected->setEnabled(enable);
    btnClearList->setEnabled(enable);
    TrackList->setEnabled(enable);
    cbFormat->setEnabled(enable);
    spMin->setEnabled(enable);
    spMax->setEnabled(enable);

    if (enable) {
        btnStart->setText(tr("St&art"));
        lblCurrentTrack->setText(QStringLiteral(""));
        TrackList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        TrackList->setSortingEnabled(true);
        TotalProgress->setValue(0);
    } else {
        btnStart->setText(tr("Stop"));
        TrackList->setSortingEnabled(false);
        TrackList->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    TrackList->clearSelection();
}

void DlgBpmDetect::slotStartStop() {
    if (innerEventLoop_ && innerEventLoop_->isRunning()) {
        // LCOV_EXCL_START
        Q_ASSERT_X(btnStart->text() == tr("Stop"), "slotStartStop", "Button text should be 'Stop'");
        slotStop();
        // LCOV_EXCL_STOP
    } else {
        Q_ASSERT_X(
            btnStart->text() == tr("St&art"), "slotStartStop", "Button text should be 'Start'");
        slotStart();
    }
}

void DlgBpmDetect::slotStart() {
    if ((innerEventLoop_ && innerEventLoop_->isRunning()) || !TrackList->topLevelItemCount()) {
        return;
    }
    enableControls(false);
    pendingTracks_ = TrackList->topLevelItemCount();
    QList<TrackItem *> items;
    for (auto i = 0; i < TrackList->topLevelItemCount(); ++i) {
        auto item = static_cast<TrackItem *>(TrackList->topLevelItem(i));
        if (chbSkipScanned->isChecked() && item->track()->hasValidBpm()) {
            --pendingTracks_;
            continue;
        }
        items.append(item);
    }
    if (!pendingTracks_) {
        enableControls(true);
        return;
    }
    TotalProgress->setMaximum(pendingTracks_);
    TotalProgress->setValue(0);
    for (const auto &item : items) {
        innerEventLoop_ = new QEventLoop(this);
        item->resetLastError();
        item->progressBar()->setValue(0);
        item->progressBar()->setTextVisible(true);
        item->track()->setDetector(detector_);
        item->track()->detectBpm();
        qCDebug(gLogBpmDetect) << "Starting inner loop. File:" << item->track()->fileName();
        if (innerEventLoop_->exec(QEventLoop::ExcludeUserInputEvents |
                                  QEventLoop::ExcludeSocketNotifiers)) {
            // LCOV_EXCL_START
            qCDebug(gLogBpmDetect) << "Inner loop exited with non-zero code. Stopping.";
            break;
            // LCOV_EXCL_STOP
        }
        qCDebug(gLogBpmDetect) << "Inner loop exited. File:" << item->track()->fileName();
    }
}

void DlgBpmDetect::slotStop() {
    if (innerEventLoop_ && innerEventLoop_->isRunning()) {
        // LCOV_EXCL_START
        qCDebug(gLogBpmDetect) << "Stopping inner loop.";
        innerEventLoop_->exit(1);
        // LCOV_EXCL_STOP
    }
    pendingTracks_ = 0;
    lblCurrentTrack->setText(QStringLiteral(""));
    for (auto i = 0; i < TrackList->topLevelItemCount(); ++i) {
        auto item = static_cast<TrackItem *>(TrackList->topLevelItem(i));
        item->progressBar()->setValue(0);
        item->progressBar()->setTextVisible(false);
    }
    enableControls(true);
}

void DlgBpmDetect::slotAddFiles(const QStringList &files) {
    if (innerEventLoop_ && innerEventLoop_->isRunning()) {
        // LCOV_EXCL_START
        return;
        // LCOV_EXCL_STOP
    }
    QStringList filteredFiles;
    for (const auto &file : files) {
        qCDebug(gLogBpmDetect) << "Checking file" << file;
        if (!isDecodableFile(file)) {
            qCDebug(gLogBpmDetect) << "File is not decodable, skipping:" << file;
            continue;
        }
        filteredFiles << file;
    }
    if (filteredFiles.size()) {
        pendingTracks_ += static_cast<int>(filteredFiles.size());
        TotalProgress->setMaximum(pendingTracks_);
    }
    auto i = 0;
    for (const auto &fileName : filteredFiles) {
        auto track = new Track(fileName, new QAudioDecoder(this), this);
        auto item = new TrackItem(TrackList, track);
        item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        auto progressBar = new QProgressBar(this);
        progressBar->setMaximum(100);
        progressBar->setMaximumHeight(15);
        item->setProgressBar(progressBar);
        TrackList->setItemWidget(item, kProgressColumn, progressBar);
        connect(track, &Track::hasBpm, [item, this, track](bpmtype bpm) {
            if (!innerEventLoop_ || !innerEventLoop_->isRunning()) {
                // LCOV_EXCL_START
                qCDebug(gLogBpmDetect)
                    << "Loop is not running. Ignoring BPM for track" << track->fileName();
                return;
                // LCOV_EXCL_STOP
            }
            qCDebug(gLogBpmDetect) << "Received BPM for track" << track->fileName();
            item->setText(0, QString::number(bpm, 'f', 2));
            if (chbSave->isChecked()) {
                item->track()->setFormat(cbFormat->currentText());
                item->track()->saveBpm();
                item->refreshSavedBpmIndicator();
                item->setLastError(getLastError());
            }
        });
        connect(track, &Track::finished, this, [track, this, progressBar]() {
            progressBar->setValue(0);
            progressBar->setTextVisible(false);
            if (!innerEventLoop_ || !innerEventLoop_->isRunning()) {
                // LCOV_EXCL_START
                qCDebug(gLogBpmDetect)
                    << "Loop not running. Ignoring finished signal for track" << track;
                return;
                // LCOV_EXCL_STOP
            }
            innerEventLoop_->quit();
            if (--pendingTracks_ == 0) {
                qCDebug(gLogBpmDetect) << "No more pending tracks, stopping.";
                slotStop();
            } else {
                TotalProgress->setValue(TotalProgress->maximum() - pendingTracks_);
            }
        });
        connect(track, &Track::progress, this, [this, progressBar](qint64 pos, qint64 length) {
            if (length > 0 && innerEventLoop_ && innerEventLoop_->isRunning()) {
                auto currentFilePercent =
                    (static_cast<double>(pos) / static_cast<double>(length)) * 100;
                progressBar->setValue(static_cast<int>(currentFilePercent));
            }
        });
        lblCurrentTrack->setText(tr("Adding %1").arg(fileName));
        TotalProgress->setValue(++i);
    }
    lblCurrentTrack->setText(QStringLiteral(""));
    auto itemCount = TrackList->topLevelItemCount();
    if (itemCount) {
        TotalProgress->setMaximum(itemCount * 100);
    } else {
        TotalProgress->setMaximum(100);
    }
    TotalProgress->reset();
    TotalProgress->setValue(0);
}

QStringList DlgBpmDetect::filesFromDir(const QString &path) const {
    QDir d(path), f(path);
    QStringList files;
    if (!d.exists(path)) {
        return files;
    }
    d.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    f.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    auto dirs = d.entryList();
    files = f.entryList();

    for (const auto &currentDir : dirs) {
        for (const auto &dirFile :
             filesFromDir(d.absolutePath() + QStringLiteral("/") + currentDir)) {
            files.append(currentDir + QStringLiteral("/") + dirFile);
        }
    }

    return files;
}

void DlgBpmDetect::slotClearTrackList() {
    TrackList->clear();
}

void DlgBpmDetect::slotClearDetected() {
    for (auto i = 0; i < TrackList->topLevelItemCount(); ++i) {
        auto item = static_cast<TrackItem *>(TrackList->topLevelItem(i));
        if (!item) {
            // LCOV_EXCL_START
            break;
            // LCOV_EXCL_STOP
        }
        if (item->track()->hasValidBpm()) {
            delete item;
            --i;
        }
    }
}

void DlgBpmDetect::slotDropped(QDropEvent *e) {
    // LCOV_EXCL_START
    if (!e) {
        return;
    }
    // LCOV_EXCL_STOP
    const auto mdata = e->mimeData();
    // LCOV_EXCL_START
    if (!mdata->hasUrls()) {
        return;
    }
    // LCOV_EXCL_STOP
    auto urllist = mdata->urls();
    e->accept();
    QStringList files;
    for (const auto &url : urllist) {
        files << url.toLocalFile();
    }
    slotAddFiles(files);
}

void DlgBpmDetect::slotSaveBpm() {
    auto items = TrackList->selectedItems();
    if (!items.size()) {
        return;
    }

    for (const auto qItem : items) {
        auto item = static_cast<TrackItem *>(qItem);
        auto track = item->track();
        track->setBpm(item->text(0).toDouble());
        track->setFormat(cbFormat->currentText());
        track->saveBpm();
        item->refreshSavedBpmIndicator();
        item->setLastError(getLastError());
    }
}

void DlgBpmDetect::setRecentPath(const QString &path) {
    recentPath_ = path;
}

QString DlgBpmDetect::recentPath() const {
    return recentPath_;
}

void DlgBpmDetect::setDetector(AbstractBpmDetector *detector) {
    detector_ = detector;
}

// LCOV_EXCL_START
void DlgBpmDetect::slotRemoveSelected() {
    TrackList->slotRemoveSelected();
}

void DlgBpmDetect::slotAddFiles() {
    QStringList files;
    files =
        QFileDialog::getOpenFileNames(this, tr("Add tracks"), recentPath(), tr("All files (*.*)"));
    if (files.size() > 0) {
        setRecentPath(files[0].left(files[0].lastIndexOf(QChar::fromLatin1('/'))));
    }
    slotAddFiles(files);
}

void DlgBpmDetect::slotAddDir() {
    auto path = QFileDialog::getExistingDirectory(this, tr("Add directory"), recentPath());

    if (path != nullptr) {
        setRecentPath(path);
        QStringList list;
        list = filesFromDir(path);
        if (!list.size()) {
            return;
        }

        if (!path.endsWith(QStringLiteral("/"))) {
            path.append(QStringLiteral("/"));
        }

        QStringList files;
        for (auto i = 0; i < list.size(); i++) {
            auto fileName = path + list[i];
            files.append(fileName);
        }

        slotAddFiles(files);
    }
}

void DlgBpmDetect::slotListMenuPopup(const QPoint &) {
    listMenu_->popup(QCursor::pos());
}

void DlgBpmDetect::slotClearBpm() {
    auto items = TrackList->selectedItems();
    if (!items.size()) {
        return;
    }

    auto clear = QMessageBox::warning(this,
                                      tr("Clear BPM"),
                                      tr("Clear BPMs of all selected tracks?"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
    if (clear == QMessageBox::No) {
        return;
    }

    for (const auto qItem : items) {
        auto item = static_cast<TrackItem *>(qItem);
        item->track()->clearBpm();
        item->setText(0, QStringLiteral("000.00"));
        item->refreshSavedBpmIndicator();
        item->setLastError(getLastError());
    }
}

void DlgBpmDetect::slotTestBpm() {
    auto item = static_cast<TrackItem *>(TrackList->currentItem());
    if (!item || !item->track()->hasValidBpm()) {
        return;
    }
    DlgTestBpm testBpmDialog(
        item->track()->fileName(),
        item->track()->bpm(),
        new DlgTestBpmPlayer(item->track()->fileName(), 4, item->track()->bpm(), 0, this));
    testBpmDialog.exec();
}

void DlgBpmDetect::slotShowAbout() {
    QMessageBox::about(this,
                       tr("About BPM Detect"),
                       tr(" Version:\t%1\n \
Description:\tAutomatic BPM (beats per minute) detection tool.\n \
License:\tGNU General Public License\n \
\n \
Authors:\tAndrew Udvare, Martin Sakmar\n \
Email:\taudvare+bpmdetect@gmail.com")
                           .arg(QCoreApplication::applicationVersion()));
}
// LCOV_EXCL_STOP
