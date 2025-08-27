// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QMimeData>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtGui/QCursor>
#include <QtGui/QDropEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "debug.h"
#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"
#include "ffmpegutils.h"
#include "qdroplistview.h"
#include "track/track.h"
#include "trackitem.h"

#define kProgressColumn 4

DlgBpmDetect::DlgBpmDetect(QWidget *parent) : QWidget(parent) {
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
    auto testBpmAction = listMenu_->addAction(tr("Test BPM"), this, &DlgBpmDetect::slotTestBpm);
#ifdef Q_OS_WIN
    // Check if Media Feature Pack is installed.
    HKEY hKey;
    auto result =
        RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\WindowsFeatures",
                      0,
                      KEY_READ,
                      &hKey);
    auto result2 =
        RegQueryValueExA(hKey, "WindowsMediaVersion", nullptr, nullptr, nullptr, nullptr);
    if (result != ERROR_SUCCESS || result2 != ERROR_SUCCESS) {
        testBpmAction->setEnabled(false);
        QMessageBox::warning(this,
                             tr("Media Feature Pack not installed"),
                             tr("The Media Feature Pack is not installed on this system. "
                                "The BPM testing feature has been disabled."));
    }
    RegCloseKey(hKey);
#else
    Q_UNUSED(testBpmAction)
#endif
    listMenu_->addSeparator();
    listMenu_->addAction(tr("Save BPM"), this, &DlgBpmDetect::slotSaveBpm);
    listMenu_->addAction(tr("Clear BPM"), this, &DlgBpmDetect::slotClearBpm);

    // Add columns to TrackList
    TrackList->setHeaderLabels(
        {tr("BPM"), tr("Artist"), tr("Title"), tr("Length"), tr("Progress"), tr("Filename")});

    TrackList->setColumnWidth(0, 60);
    TrackList->setColumnWidth(1, 200);
    TrackList->setColumnWidth(2, 200);
    TrackList->setColumnWidth(3, 60);
    TrackList->setColumnWidth(4, 100);
    TrackList->setColumnWidth(5, 400);

    connect(TrackList,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(slotListMenuPopup(const QPoint &)));
    connect(TrackList, &QDropListView::drop, this, &DlgBpmDetect::slotDropped);
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
    QSettings settings;
    QString format =
        settings.value(QStringLiteral("/BPMDetect/TBPMFormat"), QStringLiteral("0.00")).toString();
    auto skip = settings.value(QStringLiteral("/BPMDetect/SkipScanned"), true).toBool();
    auto save = settings.value(QStringLiteral("/BPMDetect/SaveBPM"), false).toBool();
    auto recentPath =
        settings.value(QStringLiteral("/BPMDetect/RecentPath"), QStringLiteral("")).toString();
    auto minBPM = settings.value(QStringLiteral("/BPMDetect/MinBPM"), 80).toInt();
    auto maxBPM = settings.value(QStringLiteral("/BPMDetect/MaxBPM"), 190).toInt();
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
        settings.value(QStringLiteral("/BPMDetect/Geometry"), saveGeometry()).toByteArray());
    move(settings.value(QStringLiteral("/BPMDetect/Position"), pos()).toPoint());
    resize(settings.value(QStringLiteral("/BPMDetect/Size"), size()).toSize());
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
        item->progressBar()->setValue(0);
        item->progressBar()->setTextVisible(true);
        item->track()->setDetector(detector_);
        if (item->track()->detectBpm() != Track::Detecting) {
            qCDebug(gLogBpmDetect)
                << "Error starting detection for track" << item->track()->fileName();
            item->progressBar()->setValue(0);
            item->progressBar()->setTextVisible(false);
            if (--pendingTracks_ == 0) {
                qCDebug(gLogBpmDetect) << "No more pending tracks, stopping.";
                slotStop();
                return;
            }
            TotalProgress->setValue(TotalProgress->maximum() - pendingTracks_);
            continue;
        }
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
        pendingTracks_ += filteredFiles.size();
        TotalProgress->setMaximum(pendingTracks_);
    }
    for (auto i = 0; i < filteredFiles.size(); ++i) {
        auto track = new Track(files[i], new QAudioDecoder(this), this);
        auto item = new TrackItem(TrackList, track);
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
        lblCurrentTrack->setText(tr("Adding %1").arg(files[i]));
        TotalProgress->setValue(i);
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
    }
}

void DlgBpmDetect::setRecentPath(const QString &path) {
    m_qRecentPath = path;
}

QString DlgBpmDetect::recentPath() const {
    return m_qRecentPath;
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
    }
}

void DlgBpmDetect::slotTestBpm() {
    auto item = TrackList->currentItem();
    if (!item) {
        return;
    }
    auto bpm = item->text(0).toFloat();
    static const auto epsilon = 0.0001;
    if (bpm <= epsilon) {
        return;
    }
    auto file = item->text(TrackList->columnCount() - 1);

    DlgTestBpm testBpmDialog(
        file, bpm, new DlgTestBpmPlayer(file, 4, bpm, new QAudioDecoder(this), 0, this));
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
