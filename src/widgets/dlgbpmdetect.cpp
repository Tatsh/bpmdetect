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

#define PROGRESSCOLUMN 4

DlgBpmDetect::DlgBpmDetect(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    setStarted(false);
    m_pCurItem = nullptr;
    m_iCurTrackIdx = 0;
    m_pProgress = nullptr;

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
    }
    RegCloseKey(hKey);
#else
    Q_UNUSED(testBpmAction)
#endif
    m_pListMenu->addSeparator();
    m_pListMenu->addAction(tr("Save BPM"), this, &DlgBpmDetect::slotSaveBpm);
    m_pListMenu->addAction(tr("Clear BPM"), this, &DlgBpmDetect::slotClearBpm);

    // Add columns to TrackList
    QStringList hLabels{
        tr("BPM"), tr("Artist"), tr("Title"), tr("Length"), tr("Progress"), tr("Filename")};
    TrackList->setHeaderLabels(hLabels);

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

    createTrackProxy(QStringLiteral(""));

    connect(&m_qTimer, &QTimer::timeout, this, &DlgBpmDetect::slotTimerDone);
    m_qTimer.start(20);
}

DlgBpmDetect::~DlgBpmDetect() {
    if (started())
        slotStop();
    saveSettings();
    delete m_pTrack;
    m_pTrack = nullptr;
}

void DlgBpmDetect::createTrackProxy(const QString &fileName) {
    if (m_pTrack) {
        delete m_pTrack;
        m_pTrack = nullptr;
    }
    m_pTrack = new TrackProxy(fileName, true);
    m_pTrack->setConsoleProgress(false);
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
    if (idx >= 0)
        cbFormat->setCurrentIndex(idx);
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
    if (started()) {
        slotStop();
    } else {
        slotStart();
    }
}

void DlgBpmDetect::slotStart() {
    if (started() || !TrackList->topLevelItemCount())
        return;

    setStarted(true);
    enableControls(false);

    TotalProgress->setMaximum(TrackList->topLevelItemCount() * 100);
    TotalProgress->setValue(0);
    m_pTrack->setMinimumBpm(spMin->value());
    m_pTrack->setMaximumBpm(spMax->value());

    slotDetectNext();
}

void DlgBpmDetect::slotStop() {
    m_pTrack->stop();
    setStarted(false);
    lblCurrentTrack->setText(QStringLiteral(""));
    enableControls(true);
}

void DlgBpmDetect::slotDetectNext(bool skipped) {
    if (!m_pCurItem) {
        // No previous item, get the first item and start
        m_pCurItem = TrackList->topLevelItem(0);
    } else {
        if (!skipped) {
            // display and save BPM
            m_pCurItem->setText(0, m_pTrack->formatted(QStringLiteral("000.00")));
            if (chbSave->isChecked())
                m_pTrack->setFormat(cbFormat->currentText());
            if (chbSave->isChecked())
                m_pTrack->saveBpm();
        }
        if (m_pCurItem) {
            // next TrackList item
            int curidx = TrackList->indexOfTopLevelItem(m_pCurItem);
            TrackList->setItemWidget(m_pCurItem, PROGRESSCOLUMN, nullptr);
            m_pProgress = nullptr;
            if (curidx >= 0)
                m_pCurItem = TrackList->topLevelItem(1 + curidx);
            else
                m_pCurItem = nullptr;
        }
    }

    if (!m_pCurItem) {
        // no next item, stop
        slotStop();
        return;
    }

    TrackList->clearSelection();
    if (m_iCurTrackIdx < 10)
        TrackList->scrollToItem(m_pCurItem, QAbstractItemView::EnsureVisible);
    else
        TrackList->scrollToItem(m_pCurItem, QAbstractItemView::PositionAtCenter);
    m_pCurItem->setSelected(true);

    QString file = m_pCurItem->text(TrackList->columnCount() - 1);
    lblCurrentTrack->setText(file.section(QChar::fromLatin1('/'), -1, -1));
    bpmtype BPM = m_pCurItem->text(0).toDouble();
    TotalProgress->setValue(100 * m_iCurTrackIdx++);
    if (chbSkipScanned->isChecked() && BPM > 0) {
        // BPM is not zero, skip this item
        slotDetectNext(true);
        return;
    }

    // start detection for current item
    m_pProgress = new QProgressBar(this);
    m_pProgress->setTextVisible(false);
    m_pProgress->setMaximum(1000);
    m_pProgress->setMaximumHeight(15);
    TrackList->setItemWidget(m_pCurItem, PROGRESSCOLUMN, m_pProgress);
    m_pTrack->setFileName(file);
    m_pTrack->setRedetect(!chbSkipScanned->isChecked());
    m_pTrack->startDetection();
}

void DlgBpmDetect::slotTimerDone() {
    if (m_pProgress)
        m_pProgress->setValue(static_cast<int>(10 * m_pTrack->progress()));
    TotalProgress->setValue(100 * (m_iCurTrackIdx - 1) + static_cast<int>(m_pTrack->progress()));
    if (started() && m_pTrack->isFinished()) {
        TotalProgress->setValue(100 * (m_iCurTrackIdx) + static_cast<int>(m_pTrack->progress()));
        slotDetectNext();
    }
}

void DlgBpmDetect::slotAddFiles(const QStringList &files) {
    if (!started() && files.size()) {
        TotalProgress->setMaximum(static_cast<int>(files.size()));
    }
    for (int i = 0; i < files.size(); ++i) {
        TrackProxy track(files[i], true);
        QStringList columns{track.formatted(QStringLiteral("000.00")),
                            track.artist(),
                            track.title(),
                            track.formattedLength(),
                            QStringLiteral(""),
                            files.at(i)};
        if (!started()) {
            lblCurrentTrack->setText(tr("Adding %1").arg(files.at(i)));
            TotalProgress->setValue(i);
        }
        new QTreeWidgetItem(TrackList, columns);
        qApp->processEvents();
    }
    if (!started()) {
        lblCurrentTrack->setText(QStringLiteral(""));
        int itemcount = TrackList->topLevelItemCount();
        if (itemcount)
            TotalProgress->setMaximum(itemcount * 100);
        else
            TotalProgress->setMaximum(100);
        TotalProgress->reset();
        TotalProgress->setValue(0);
    }
}

void DlgBpmDetect::slotAddFiles() {
    QStringList files;
    files = QFileDialog::getOpenFileNames(
        this,
        tr("Add tracks"),
        recentPath(),
        tr("Audio files (*.wav *.mp3 *.ogg *.flac *.wv *.wavpack *.fla *.flc);;All files (*.*)"));
    if (files.size() > 0)
        setRecentPath(files[0].left(files[0].lastIndexOf(QChar::fromLatin1('/'))));
    slotAddFiles(files);
}

void DlgBpmDetect::slotAddDir() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Add directory"), recentPath());

    if (path != nullptr) {
        setRecentPath(path);
        QStringList list;
        list = filesFromDir(path);
        if (list.size() == 0)
            return;

        if (!path.endsWith(QStringLiteral("/")))
            path.append(QStringLiteral("/"));

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

/**
 * @brief Get all wav, ogg, flac and mp3 files
 * from directory path including files from subdirectories
 * @param path path from which the files are added
 * @return QStringList of files with relative paths
 */
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

void DlgBpmDetect::slotTestBpm() {
    auto item = TrackList->currentItem();
    if (!item)
        return;
    float bpm = item->text(0).toFloat();
    if (bpm == 0.0f)
        return;
    auto file = item->text(TrackList->columnCount() - 1);

    DlgTestBpm testBpmDialog(
        file, bpm, new DlgTestBpmPlayer(file, 4, bpm, new QAudioDecoder(this), 0, this));
    testBpmDialog.exec();
}

// LCOV_EXCL_START
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

void DlgBpmDetect::slotClearTrackList() {
    TrackList->clear();
    delete m_pProgress;
    m_pProgress = nullptr;
}

void DlgBpmDetect::slotClearDetected() {
    for (auto i = 0; i < TrackList->topLevelItemCount(); ++i) {
        auto item = TrackList->topLevelItem(i);
        if (!item)
            break;

        float fBpm = item->text(0).toFloat();
        if (fBpm > 0) {
            delete item;
            i--;
        }
    }
}

void DlgBpmDetect::slotDropped(QDropEvent *e) {
    if (!e)
        return;
    const QMimeData *mdata = e->mimeData();
    if (!mdata->hasUrls())
        return;
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
    if (!items.size())
        return;

    for (int i = 0; i < items.size(); ++i) {
        auto item = items.at(i);
        TrackProxy track(item->text(TrackList->columnCount() - 1));
        track.setBpm(item->text(0).toDouble());
        track.setFormat(cbFormat->currentText());
        track.saveBpm();
    }
}

void DlgBpmDetect::slotClearBpm() {
    auto items = TrackList->selectedItems();
    if (!items.size())
        return;

    int clear = QMessageBox::warning(this,
                                     tr("Clear BPM"),
                                     tr("Clear BPMs of all selected tracks?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if (clear == QMessageBox::No)
        return;

    for (int i = 0; i < items.size(); ++i) {
        auto item = items.at(i);
        TrackProxy track(item->text(TrackList->columnCount() - 1));
        track.clearBpm();
        item->setText(0, QStringLiteral("000.00"));
    }
}

void DlgBpmDetect::setStarted(bool started) {
    m_bStarted = started;
}

bool DlgBpmDetect::started() const {
    return m_bStarted;
}

void DlgBpmDetect::setRecentPath(const QString &path) {
    m_qRecentPath = path;
}

QString DlgBpmDetect::recentPath() const {
    return m_qRecentPath;
}
