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

#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"
#include "qdroplistview.h"
#include "track/track.h"
#include "trackitem.h"

#define kProgressColumn 4

DlgBpmDetect::DlgBpmDetect(QWidget *parent) : QWidget(parent) {
    setupUi(this);

    loadSettings();

    // Create TrackList menu
    m_pListMenu = new QMenu(TrackList);
    m_pListMenu->addAction(tr("Add files"), [this]() { slotAddFiles(); });
    m_pListMenu->addAction(tr("Add directory"), this, &DlgBpmDetect::slotAddDir);
    m_pListMenu->addSeparator();
    m_pListMenu->addAction(tr("Remove selected tracks"), this, &DlgBpmDetect::slotRemoveSelected);
    m_pListMenu->addAction(tr("Remove tracks with BPM"), this, &DlgBpmDetect::slotClearDetected);
    m_pListMenu->addAction(tr("Clear list"), this, &DlgBpmDetect::slotClearTrackList);
    m_pListMenu->addSeparator();
    auto testBpmAction = m_pListMenu->addAction(tr("Test BPM"), this, &DlgBpmDetect::slotTestBpm);
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
    m_pListMenu->addSeparator();
    m_pListMenu->addAction(tr("Save BPM"), this, &DlgBpmDetect::slotSaveBpm);
    m_pListMenu->addAction(tr("Clear BPM"), this, &DlgBpmDetect::slotClearBpm);

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
    if (m_bStarted) {
        slotStop();
    }
    saveSettings();
}

void DlgBpmDetect::loadSettings() {
    QSettings settings;
    QString format =
        settings.value(QStringLiteral("/BPMDetect/TBPMFormat"), QStringLiteral("0.00")).toString();
    bool skip = settings.value(QStringLiteral("/BPMDetect/SkipScanned"), true).toBool();
    bool save = settings.value(QStringLiteral("/BPMDetect/SaveBPM"), false).toBool();
    QString recentPath =
        settings.value(QStringLiteral("/BPMDetect/RecentPath"), QStringLiteral("")).toString();
    int minBPM = settings.value(QStringLiteral("/BPMDetect/MinBPM"), 80).toInt();
    int maxBPM = settings.value(QStringLiteral("/BPMDetect/MaxBPM"), 190).toInt();
    chbSkipScanned->setChecked(skip);
    chbSave->setChecked(save);
    int idx = cbFormat->findText(format);
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
        btnStart->setText(tr("Start"));
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
    m_pCurItem = nullptr;
    m_iCurTrackIdx = 0;
}

void DlgBpmDetect::slotStartStop() {
    if (m_bStarted) {
        slotStop();
    } else {
        slotStart();
    }
}

void DlgBpmDetect::slotStart() {
    if (m_bStarted || !TrackList->topLevelItemCount()) {
        return;
    }

    m_bStarted = true;
    enableControls(false);

    TotalProgress->setMaximum(TrackList->topLevelItemCount() * 100);
    TotalProgress->setValue(0);
    if (m_pCurItem) {
        m_pCurItem->track()->setMinimumBpm(spMin->value());
        m_pCurItem->track()->setMaximumBpm(spMax->value());
    }

    slotDetectNext();
}

void DlgBpmDetect::slotStop() {
    if (m_pCurItem) {
        m_pCurItem->track()->stop();
    }
    m_bStarted = false;
    lblCurrentTrack->setText(QStringLiteral(""));
    enableControls(true);
}

void DlgBpmDetect::slotDetectNext(bool skipped) {
    if (!m_pCurItem) {
        // No previous item, get the first item and start
        m_pCurItem = static_cast<TrackItem *>(TrackList->topLevelItem(0));
    } else {
        if (!skipped) {
            // display and save BPM
            m_pCurItem->setText(0, m_pCurItem->track()->formatted(QStringLiteral("000.00")));
            if (chbSave->isChecked())
                m_pCurItem->track()->setFormat(cbFormat->currentText());
            if (chbSave->isChecked())
                m_pCurItem->track()->saveBpm();
        }
        if (m_pCurItem) {
            // next TrackList item
            int curidx = TrackList->indexOfTopLevelItem(m_pCurItem);
            TrackList->setItemWidget(m_pCurItem, kProgressColumn, nullptr);
            m_pProgress = nullptr;
            if (curidx >= 0) {
                m_pCurItem = static_cast<TrackItem *>(TrackList->topLevelItem(1 + curidx));
            } else {
                m_pCurItem = nullptr;
            }
        }
    }

    if (!m_pCurItem) {
        // no next item, stop
        slotStop();
        return;
    }

    TrackList->clearSelection();
    if (m_iCurTrackIdx < 10) {
        TrackList->scrollToItem(m_pCurItem, QAbstractItemView::EnsureVisible);
    } else {
        TrackList->scrollToItem(m_pCurItem, QAbstractItemView::PositionAtCenter);
    }
    m_pCurItem->setSelected(true);

    auto file = m_pCurItem->text(TrackList->columnCount() - 1);
    lblCurrentTrack->setText(file.section(QChar::fromLatin1('/'), -1, -1));
    auto BPM = m_pCurItem->text(0).toDouble();
    TotalProgress->setValue(100 * m_iCurTrackIdx++);
    if (chbSkipScanned->isChecked() && BPM > 0) {
        // BPM is not zero, skip this item
        slotDetectNext(true);
        return;
    }

    // start detection for current item
    m_pProgress = new QProgressBar(this);
    m_pProgress->setTextVisible(false);
    m_pProgress->setMaximum(100);
    m_pProgress->setMaximumHeight(15);
    TrackList->setItemWidget(m_pCurItem, kProgressColumn, m_pProgress);
    m_pCurItem->track()->setRedetect(!chbSkipScanned->isChecked());
    m_pCurItem->track()->setDetector(m_pDetector);
    m_pCurItem->track()->detectBpm();
}

void DlgBpmDetect::slotAddFiles(const QStringList &files) {
    if (!m_bStarted && files.size()) {
        TotalProgress->setMaximum(static_cast<int>(files.size()));
    }
    for (int i = 0; i < files.size(); ++i) {
        auto track = new Track(files[i], &decoder_, true, this);
        auto item = new TrackItem(TrackList, track);
        connect(track, &Track::hasBpm, [item, this](bpmtype bpm) {
            item->setText(0, QString::number(bpm, 'f', 2));
            slotDetectNext();
        });
        connect(track, &Track::hasLength, this, [track, item](quint64 ms) {
            item->setText(3, track->formattedLength());
        });
        connect(track, &Track::progress, this, [this](qint64 pos, qint64 length) {
            if (m_pProgress && length > 0) {
                auto currentFilePercent =
                    100 * (static_cast<double>(pos) / static_cast<double>(length));
                m_pProgress->setValue(static_cast<int>(currentFilePercent));
                TotalProgress->setValue(100 * (m_iCurTrackIdx - 1) + currentFilePercent);
            }
        });
        if (!m_bStarted) {
            lblCurrentTrack->setText(tr("Adding %1").arg(files.at(i)));
            TotalProgress->setValue(i);
        }
        qApp->processEvents();
    }
    if (!m_bStarted) {
        lblCurrentTrack->setText(QStringLiteral(""));
        int itemcount = TrackList->topLevelItemCount();
        if (itemcount) {
            TotalProgress->setMaximum(itemcount * 100);
        } else {
            TotalProgress->setMaximum(100);
        }
        TotalProgress->reset();
        TotalProgress->setValue(0);
    }
}

QStringList DlgBpmDetect::filesFromDir(const QString &path) const {
    QDir d(path), f(path);
    QStringList files;
    if (!d.exists(path))
        return files;
    d.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
    f.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    f.setNameFilters({QStringLiteral("*.fla"),
                      QStringLiteral("*.flac"),
                      QStringLiteral("*.flc"),
                      QStringLiteral("*.mp3"),
                      QStringLiteral("*.ogg"),
                      QStringLiteral("*.wav"),
                      QStringLiteral("*.wavpack"),
                      QStringLiteral("*.wv")});

    QStringList dirs = d.entryList(QDir::NoDotAndDotDot);
    files = f.entryList();

    for (int i = 0; i < dirs.size(); ++i) {
        QString cdir = dirs[i];
        QStringList dfiles = filesFromDir(d.absolutePath() + QStringLiteral("/") + cdir);
        for (int j = 0; j < dfiles.size(); ++j) {
            files.append(cdir + QStringLiteral("/") + dfiles[j]);
        }
    }

    return files;
}

void DlgBpmDetect::slotRemoveSelected() {
    TrackList->slotRemoveSelected();
}

void DlgBpmDetect::slotClearTrackList() {
    TrackList->clear();
    delete m_pProgress;
    m_pProgress = nullptr;
}

void DlgBpmDetect::slotClearDetected() {
    for (auto i = 0; i < TrackList->topLevelItemCount(); ++i) {
        auto item = TrackList->topLevelItem(i);
        if (!item) {
            break;
        }

        float fBpm = item->text(0).toFloat();
        if (fBpm > 0) {
            delete item;
            i--;
        }
    }
}

void DlgBpmDetect::slotDropped(QDropEvent *e) {
    // LCOV_EXCL_START
    if (!e) {
        return;
    }
    // LCOV_EXCL_STOP
    const QMimeData *mdata = e->mimeData();
    // LCOV_EXCL_START
    if (!mdata->hasUrls()) {
        return;
    }
    // LCOV_EXCL_STOP
    QList<QUrl> urllist = mdata->urls();
    e->accept();
    QStringList files;
    for (int i = 0; i < urllist.size(); ++i) {
        files << urllist[i].toLocalFile();
    }
    slotAddFiles(files);
}

void DlgBpmDetect::slotSaveBpm() {
    auto items = TrackList->selectedItems();
    // LCOV_EXCL_START
    if (!items.size()) {
        return;
    }
    // LCOV_EXCL_STOP

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
    m_pDetector = detector;
}

// LCOV_EXCL_START
void DlgBpmDetect::slotAddFiles() {
    QStringList files;
    files = QFileDialog::getOpenFileNames(
        this,
        tr("Add tracks"),
        recentPath(),
        tr("Audio files (*.wav *.mp3 *.ogg *.flac *.wv *.wavpack *.fla *.flc);;All files (*.*)"));
    if (files.size() > 0) {
        setRecentPath(files[0].left(files[0].lastIndexOf(QChar::fromLatin1('/'))));
    }
    slotAddFiles(files);
}

void DlgBpmDetect::slotAddDir() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Add directory"), recentPath());

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
            QString fileName = path + list[i];
            files.append(fileName);
        }

        slotAddFiles(files);
    }
}

void DlgBpmDetect::slotListMenuPopup(const QPoint &) {
    m_pListMenu->popup(QCursor::pos());
}

void DlgBpmDetect::slotClearBpm() {
    auto items = TrackList->selectedItems();
    if (!items.size()) {
        return;
    }

    int clear = QMessageBox::warning(this,
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
    float bpm = item->text(0).toFloat();
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
