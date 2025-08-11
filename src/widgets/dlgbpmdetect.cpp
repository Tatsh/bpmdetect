// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCursor>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCore>
#include <QtGui>

#include "dlgbpmdetect.h"
#include "dlgtestbpm.h"
#include "images.h"
#include "qdroplistview.h"

#define PROGRESSCOLUMN 4

extern const char *version;

DlgBPMDetect::DlgBPMDetect(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    setStarted(false);
    m_pCurItem = nullptr;
    m_iCurTrackIdx = 0;
    m_pProgress = nullptr;

    QImage img;
    img.loadFromData(icon_png, sizeof(icon_png), "PNG");
    setWindowIcon(QPixmap::fromImage(img));
    QString strcaption = QStringLiteral("BPM Detect v");
    strcaption.append(QString::fromLocal8Bit(version));
    setWindowTitle(strcaption);

    loadSettings();

    /// Create TrackList menu
    m_pListMenu = new QMenu(TrackList);
    m_pListMenu->addAction(tr("Add files"), this, SLOT(slotAddFiles()));
    m_pListMenu->addAction(tr("Add directory"), this, SLOT(slotAddDir()));
    m_pListMenu->addSeparator();
    m_pListMenu->addAction(tr("Remove selected tracks"), this, SLOT(slotRemoveSelected()));
    m_pListMenu->addAction(tr("Remove tracks with BPM"), this, SLOT(slotClearDetected()));
    m_pListMenu->addAction(tr("Clear list"), this, SLOT(slotClearTrackList()));
    m_pListMenu->addSeparator();
#ifndef Q_OS_MACOS
    m_pListMenu->addAction(tr("Test BPM"), this, SLOT(slotTestBPM()));
#endif
    m_pListMenu->addSeparator();
    m_pListMenu->addAction(tr("Save BPM"), this, SLOT(slotSaveBPM()));
    m_pListMenu->addAction(tr("Clear BPM"), this, SLOT(slotClearBPM()));

    /// Add columns to TrackList
    QStringList hlabels;
    hlabels << tr("BPM") << tr("Artist") << tr("Title") << tr("Length") << tr("Progress")
            << tr("Filename");
    TrackList->setHeaderLabels(hlabels);

    TrackList->setColumnWidth(0, 60);
    TrackList->setColumnWidth(1, 200);
    TrackList->setColumnWidth(2, 200);
    TrackList->setColumnWidth(3, 60);
    TrackList->setColumnWidth(4, 100);
    TrackList->setColumnWidth(5, 400);

    /// Connect signals with slots
    connect(TrackList,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(slotListMenuPopup(const QPoint &)));
    connect(TrackList, SIGNAL(drop(QDropEvent *)), this, SLOT(slotDropped(QDropEvent *)));

    connect(btnStart, SIGNAL(clicked()), this, SLOT(slotStartStop()));

    m_pTrack = new TrackProxy(QStringLiteral(""));
    m_pTrack->enableConsoleProgress(false);

    connect(&m_qTimer, SIGNAL(timeout()), this, SLOT(slotTimerDone()));
    m_qTimer.start(20);
}

DlgBPMDetect::~DlgBPMDetect() {
    if (getStarted())
        slotStop();
    saveSettings();
    delete m_pTrack;
}

void DlgBPMDetect::loadSettings() {
#ifdef DEBUG
    qDebug("Loading settings");
#endif
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QStringLiteral(""),
                       QStringLiteral("BPMDetect"));
    QString format =
        settings.value(QStringLiteral("/BPMDetect/TBPMFormat"), QStringLiteral("0.00")).toString();
    bool skip = settings.value(QStringLiteral("/BPMDetect/SkipScanned"), true).toBool();
    bool save = settings.value(QStringLiteral("/BPMDetect/SaveBPM"), true).toBool();
    QString recentpath =
        settings.value(QStringLiteral("/BPMDetect/RecentPath"), QStringLiteral("")).toString();
    int minBPM = settings.value(QStringLiteral("/BPMDetect/MinBPM"), 80).toInt();
    int maxBPM = settings.value(QStringLiteral("/BPMDetect/MaxBPM"), 190).toInt();
    chbSkipScanned->setChecked(skip);
    chbSave->setChecked(save);
    int idx = cbFormat->findText(format);
    if (idx >= 0)
        cbFormat->setCurrentIndex(idx);
    setRecentPath(recentpath);
    spMin->setValue(minBPM);
    spMax->setValue(maxBPM);
}

void DlgBPMDetect::saveSettings() {
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QStringLiteral(""),
                       QStringLiteral("BPMDetect"));
    settings.setValue(QStringLiteral("/BPMDetect/TBPMFormat"), cbFormat->currentText());
    settings.setValue(QStringLiteral("/BPMDetect/SkipScanned"), chbSkipScanned->isChecked());
    settings.setValue(QStringLiteral("/BPMDetect/SaveBPM"), chbSave->isChecked());
    settings.setValue(QStringLiteral("/BPMDetect/RecentPath"), getRecentPath());
    settings.setValue(QStringLiteral("/BPMDetect/MinBPM"), spMin->value());
    settings.setValue(QStringLiteral("/BPMDetect/MaxBPM"), spMax->value());
#ifdef DEBUG
    qDebug("Settings saved");
#endif
}

/**
 ** Enable or disable controls
 ** @param enable true to enable, false to disable
 **/
void DlgBPMDetect::enableControls(bool enable) {
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

void DlgBPMDetect::slotStartStop() {
    if (getStarted()) {
        slotStop();
    } else {
        slotStart();
    }
}

void DlgBPMDetect::slotStart() {
    if (getStarted() || !TrackList->topLevelItemCount())
        return;

    setStarted(true);
    enableControls(false);

    TotalProgress->setMaximum(TrackList->topLevelItemCount() * 100);
    TotalProgress->setValue(0);
    m_pTrack->setMinBPM(spMin->value());
    m_pTrack->setMaxBPM(spMax->value());

    slotDetectNext();
}

void DlgBPMDetect::slotStop() {
    m_pTrack->stop();
    setStarted(false);
    lblCurrentTrack->setText(QStringLiteral(""));
    enableControls(true);
}

/// Start detection on next track in the list
/// @param skipped true if previous track was skipped, so BPM won't be saved
void DlgBPMDetect::slotDetectNext(bool skipped) {
    if (!m_pCurItem) {
        // No previous item, get the first item and start
        m_pCurItem = TrackList->topLevelItem(0);
    } else {
        if (!skipped) {
            // display and save BPM
            m_pCurItem->setText(0, m_pTrack->strBPM(QString::fromUtf8("000.00")));
            if (chbSave->isChecked())
                m_pTrack->setFormat(cbFormat->currentText());
            if (chbSave->isChecked())
                m_pTrack->saveBPM();
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
    double BPM = m_pCurItem->text(0).toDouble();
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
    m_pTrack->setFilename(file);
    m_pTrack->setRedetect(!chbSkipScanned->isChecked());
    m_pTrack->startDetection();
}

void DlgBPMDetect::slotTimerDone() {
    if (m_pProgress)
        m_pProgress->setValue(static_cast<int>(10 * m_pTrack->progress()));
    TotalProgress->setValue(100 * (m_iCurTrackIdx - 1) + static_cast<int>(m_pTrack->progress()));
    if (getStarted() && m_pTrack->isFinished()) {
        TotalProgress->setValue(100 * (m_iCurTrackIdx) + static_cast<int>(m_pTrack->progress()));
        slotDetectNext();
    }
}

void DlgBPMDetect::slotAddFiles(QStringList &files) {
    if (!getStarted() && files.size()) {
        TotalProgress->setMaximum(static_cast<int>(files.size()));
    }
    for (int i = 0; i < files.size(); ++i) {
        TrackProxy track(files[i], true);
        QStringList columns;
        columns << track.strBPM(QString::fromUtf8("000.00"));
        columns << track.artist();
        columns << track.title();
        columns << track.strLength();
        columns << QStringLiteral("");
        columns << files.at(i);
        if (!getStarted()) {
            lblCurrentTrack->setText(tr("Adding %1").arg(files.at(i)));
            TotalProgress->setValue(i);
        }
        new QTreeWidgetItem(TrackList, columns);
        qApp->processEvents();
    }
    if (!getStarted()) {
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

void DlgBPMDetect::slotAddFiles() {
    QStringList files;
    files = QFileDialog::getOpenFileNames(
        this,
        tr("Add tracks"),
        getRecentPath(),
        tr("Audio files (*.wav *.mp3 *.ogg *.flac);;All files (*.*)"));
    if (files.size() > 0)
        setRecentPath(files[0].left(files[0].lastIndexOf(QChar::fromLatin1('/'))));
    slotAddFiles(files);
}

void DlgBPMDetect::slotAddDir() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Add directory"), getRecentPath());

    if (path != nullptr) {
        setRecentPath(path);
        QStringList list;
        list = filesFromDir(path);
        if (list.size() == 0)
            return;

        if (!path.endsWith(QStringLiteral("/")))
            path.append(QStringLiteral("/"));

        QStringList files;
        for (uint i = 0; i < list.size(); i++) {
            QString filename = path + list[i];
            files.append(filename);
        }

        slotAddFiles(files);
    }
}

void DlgBPMDetect::slotListMenuPopup(const QPoint &) {
    m_pListMenu->popup(QCursor::pos());
}

/**
 * @brief Get all wav, ogg, flac and mp3 files
 * from directory path including files from subdirectories
 * @param path path from which the files are added
 * @return QStringList of files with relative paths
 */
QStringList DlgBPMDetect::filesFromDir(QString path) {
    QDir d(path), f(path);
    QStringList files;
    if (!d.exists(path))
        return files;
    d.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
    f.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QString nameFilters = QStringLiteral("*.wav:*.mp3:*.ogg:*.flac");
    f.setNameFilters(nameFilters.split(QChar::fromLatin1(':')));

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

void DlgBPMDetect::slotRemoveSelected() {
    TrackList->slotRemoveSelected();
}

void DlgBPMDetect::slotTestBPM() {
#ifndef Q_OS_MACOS
    QTreeWidgetItem *item = TrackList->currentItem();
    if (!item)
        return;
    float bpm = item->text(0).toFloat();
    if (bpm == 0.0f)
        return;

    DlgTestBPM tbpmd(item->text(TrackList->columnCount() - 1), bpm, this);
    tbpmd.exec();
#endif
}

void DlgBPMDetect::slotShowAbout() {
    QString description = tr("Automatic BPM (beats per minute) detecting application.");
    QString abouttext = tr(" \
Version:    \t%1 \n \
Description:\t%2 \n \
License:    \tGNU General Public License \n \
\n \
Author:     \tMartin Sakmar \n \
e-mail:     \tmartin.sakmar@gmail.com \n \
")
                            .arg(QString::fromLocal8Bit(version))
                            .arg(description);
    QMessageBox::about(this, tr("About BPM Detect"), abouttext);
}

void DlgBPMDetect::slotClearTrackList() {
    TrackList->clear();
}

void DlgBPMDetect::slotClearDetected() {
    for (int i = 0; i < TrackList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = TrackList->topLevelItem(i);
        if (!item)
            break;

        float fBPM = item->text(0).toFloat();
        if (fBPM > 0) {
            delete item;
            i--;
        }
    }
}

void DlgBPMDetect::slotDropped(QDropEvent *e) {
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

void DlgBPMDetect::slotSaveBPM() {
    QList<QTreeWidgetItem *> items = TrackList->selectedItems();
    if (!items.size())
        return;

    for (int i = 0; i < items.size(); ++i) {
        QTreeWidgetItem *item = items.at(i);
        TrackProxy track(item->text(TrackList->columnCount() - 1));
        track.setBPM(item->text(0).toDouble());
        track.setFormat(cbFormat->currentText());
        track.saveBPM();
    }
}

void DlgBPMDetect::slotClearBPM() {
    QList<QTreeWidgetItem *> items = TrackList->selectedItems();
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
        QTreeWidgetItem *item = items.at(i);
        TrackProxy track(item->text(TrackList->columnCount() - 1));
        track.clearBPM();
        item->setText(0, QStringLiteral("000.00"));
    }
}

void DlgBPMDetect::setStarted(bool started) {
    m_bStarted = started;
}

bool DlgBPMDetect::getStarted() const {
    return m_bStarted;
}

void DlgBPMDetect::setRecentPath(QString path) {
    m_qRecentPath = path;
}

QString DlgBPMDetect::getRecentPath() const {
    return m_qRecentPath;
}
