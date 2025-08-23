#include <QtTest>

#include "track/track.h"
#include "widgets/dlgbpmdetect.h"
#include "widgets/trackitem.h"

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
    void testSlotClearTrackList();
    void testSlotStartStop();
    void testSlotAddFiles();
    void testSlotDropped();
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
    QStringList files = dlg.filesFromDir(QStringLiteral("."));
    QVERIFY(files.isEmpty());
}

void DlgBpmDetectTest::testSlotStartStop() {
    DlgBpmDetect dlg;
    dlg.slotStartStop();

    auto item = new TrackItem(nullptr, new Track(this));
    item->setText(0, QStringLiteral("test.mp3"));
    dlg.TrackList->addTopLevelItem(item);
    dlg.slotStartStop();
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
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 1);
}

void DlgBpmDetectTest::testSlotDropped() {
    DlgBpmDetect dlg;
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(QStringLiteral("test.mp3")));
    mimeData->setUrls(urls);
    QDropEvent dropEvent(QPoint(10, 10), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    dlg.slotDropped(&dropEvent);
    QCOMPARE(dlg.TrackList->topLevelItemCount(), 1);
}

QTEST_MAIN(DlgBpmDetectTest)

#include "dlgbpmdetecttest.moc"
