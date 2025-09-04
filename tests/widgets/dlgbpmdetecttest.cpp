#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include <QtMultimedia/QAudioDecoder>
#include <QtTest/QtTest>

#include "ffmpegutils.h"
#include "track/abstractbpmdetector.h"
#include "track/track.h"
#include "widgets/dlgbpmdetect.h"
#include "widgets/trackitem.h"

class DummyBpmDetector : public AbstractBpmDetector {
    Q_OBJECT
public:
    DummyBpmDetector(QObject *parent = nullptr) : AbstractBpmDetector(parent) {
    }
    void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) override {
        Q_UNUSED(samples)
        Q_UNUSED(numSamples)
    }
    bpmtype getBpm() const override {
        return 120.0;
    }
    void reset() override {
    }
};

class DlgBpmDetectTest : public QObject {
    Q_OBJECT
public:
    explicit DlgBpmDetectTest(QObject *parent = nullptr);
    ~DlgBpmDetectTest() override;

private Q_SLOTS:
    void testConstructor();
    void testEnableControls();
    void testFilesFromDir();
    void testSetRecentPath();
    void testSlotAddFiles();
    void testSlotClearDetected();
    void testSlotClearTrackList();
    void testSlotDropped();
    void testSlotSaveBpm();
    void testSlotStartStop();
    void testSlotStartStopSavesBpmIfSaveIsChecked();
    void testSlotStartStopSkipsFilesWithValidBpmIfSkipScannedIsChecked();
};

DlgBpmDetectTest::DlgBpmDetectTest(QObject *parent) : QObject(parent) {
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::tempPath());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QDir::tempPath());
}

DlgBpmDetectTest::~DlgBpmDetectTest() {
}

void DlgBpmDetectTest::testConstructor() {
    DlgBpmDetect dlg;
    QVERIFY(dlg.listMenu_ != nullptr);
}

void DlgBpmDetectTest::testEnableControls() {
    DlgBpmDetect dlg;
    dlg.enableControls(true);
    QVERIFY(dlg.btnAddFiles->isEnabled());
    dlg.enableControls(false);
    QVERIFY(!dlg.btnAddFiles->isEnabled());
}

void DlgBpmDetectTest::testSetRecentPath() {
    DlgBpmDetect dlg;
    dlg.setRecentPath(QStringLiteral("/tmp"));
    QCOMPARE(dlg.recentPath(), QStringLiteral("/tmp"));
}

void DlgBpmDetectTest::testFilesFromDir() {
    DlgBpmDetect dlg;
    QCOMPARE(dlg.filesFromDir(QStringLiteral("/nonexistent-path")).size(), 0);

    QVERIFY(QDir(QStringLiteral("testdir")).removeRecursively());
    QVERIFY(QDir().mkdir(QStringLiteral("testdir")));
    QFileInfo fi(QStringLiteral("testdir"));
    QVERIFY(fi.exists());
    QVERIFY(fi.isDir());
    QVERIFY(QDir(QStringLiteral("testdir")).mkdir(QStringLiteral("subdir")));
    QFile file1(QStringLiteral("testdir/file1.txt"));
    QVERIFY(file1.open(QIODevice::WriteOnly));
    file1.close();
    QFile file2(QStringLiteral("testdir/subdir/file2.mp3"));
    QVERIFY(file2.open(QIODevice::WriteOnly));
    file2.close();
    auto files = dlg.filesFromDir(fi.absoluteFilePath());
    QCOMPARE(files.size(), 2);
}

void DlgBpmDetectTest::testSlotStartStop() {
    DlgBpmDetect dlg;
    dlg.setDetector(new DummyBpmDetector(&dlg));
    dlg.slotStartStop();
    QCOMPARE(dlg.pendingTracks_, 0);

    dlg.chbSkipScanned->setChecked(false);
    dlg.chbSave->setChecked(false);
    dlg.slotAddFiles({QString::fromUtf8(TEST_FILE), QString::fromUtf8(TEST_FILE)});
    dlg.slotStartStop();
    QCOMPARE(dlg.pendingTracks_, 0);
}

void DlgBpmDetectTest::testSlotClearTrackList() {
    DlgBpmDetect dlg;
    auto item = new TrackItem(nullptr, new Track(this));
    item->setText(0, QStringLiteral("test.mp3"));
    dlg.TrackList->addTopLevelItem(item);
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 1);
    dlg.slotClearTrackList();
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 0);
    QCOMPARE(dlg.TotalProgress->value(), 0);
}

void DlgBpmDetectTest::testSlotAddFiles() {
    DlgBpmDetect dlg;
    dlg.slotAddFiles(QStringList{QStringLiteral("test.mp3")});
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 0);
}

void DlgBpmDetectTest::testSlotDropped() {
    DlgBpmDetect dlg;
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(QStringLiteral("test.mp3")));
    mimeData->setUrls(urls);
    QDropEvent dropEvent(QPoint(10, 10), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    dlg.slotDropped(&dropEvent);
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 0);
}

void DlgBpmDetectTest::testSlotStartStopSkipsFilesWithValidBpmIfSkipScannedIsChecked() {
    DlgBpmDetect dlg;
    dlg.setDetector(new DummyBpmDetector(&dlg));

    auto item1 = new TrackItem(dlg.TrackList, new Track(this));
    item1->setProgressBar(new QProgressBar(&dlg));
    item1->setText(0, QStringLiteral("test1.mp3"));
    item1->track()->dBpm_ = 120.0;
    dlg.TrackList->addTopLevelItem(item1);

    dlg.chbSkipScanned->setChecked(true);
    dlg.slotStartStop();
    QCOMPARE(dlg.pendingTracks_, 0);
    QVERIFY(dlg.btnAddFiles->isEnabled());
}

void DlgBpmDetectTest::testSlotStartStopSavesBpmIfSaveIsChecked() {
    DlgBpmDetect dlg;
    dlg.setDetector(new DummyBpmDetector(&dlg));

    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/XXXXXX.ogg"));
    QFile originalFile(QString::fromUtf8(TEST_FILE));

    QVERIFY(tempFile.open());
    QVERIFY(originalFile.open(QIODevice::ReadOnly));
    tempFile.write(originalFile.readAll());
    tempFile.close();
    originalFile.close();

    dlg.slotAddFiles({tempFile.fileName()});
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 1);

    dlg.chbSave->setChecked(true);
    dlg.slotStartStop();
    QCOMPARE(dlg.pendingTracks_, 0);

    auto map = readTagsFromFile(tempFile.fileName());
    auto setBpm = map[QStringLiteral("bpm")].toDouble();
    qDebug() << "BPM in file:" << setBpm;
}

void DlgBpmDetectTest::testSlotClearDetected() {
    DlgBpmDetect dlg;

    auto item1 = new TrackItem(dlg.TrackList, new Track(this));
    item1->setText(1, QStringLiteral("test1.mp3"));
    item1->track()->setBpm(120.0);
    dlg.TrackList->addTopLevelItem(item1);

    auto item2 = new TrackItem(dlg.TrackList, new Track(this));
    item2->setText(1, QStringLiteral("test2.mp3"));
    dlg.TrackList->addTopLevelItem(item2);

    QCOMPARE(dlg.TrackList->topLevelItemCount(), 2);
    dlg.slotClearDetected();
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 1);
    QCOMPARE(dlg.TrackList->topLevelItem(0)->text(1), QStringLiteral("test2.mp3"));
}

void DlgBpmDetectTest::testSlotSaveBpm() {
    DlgBpmDetect dlg;
    dlg.slotSaveBpm();

    auto item1 = new TrackItem(dlg.TrackList, new Track(this));
    item1->setText(0, QStringLiteral("120.0"));
    item1->track()->dBpm_ = 0.0;
    dlg.TrackList->addTopLevelItem(item1);
    dlg.TrackList->setCurrentItem(item1);

    dlg.slotSaveBpm();
    QCOMPARE(item1->track()->bpm(), 120.0);
}

QTEST_MAIN(DlgBpmDetectTest)

#include "dlgbpmdetecttest.moc"
