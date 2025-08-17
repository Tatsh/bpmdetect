#include <QtTest>

#include "widgets/dlgbpmdetect.h"

class DlgBpmDetectTest : public QObject {
    Q_OBJECT
public:
    explicit DlgBpmDetectTest(QObject *parent = nullptr);
    ~DlgBpmDetectTest() override;

private Q_SLOTS:
    void testConstructor();
    void testEnableControls();
    void testSetStarted();
    void testSetRecentPath();
    void testFilesFromDir();
};

DlgBpmDetectTest::DlgBpmDetectTest(QObject *parent) : QObject(parent) {
}

DlgBpmDetectTest::~DlgBpmDetectTest() {
}

void DlgBpmDetectTest::testConstructor() {
    DlgBpmDetect dlg;
    QVERIFY(dlg.m_pListMenu != nullptr);
}

void DlgBpmDetectTest::testEnableControls() {
    DlgBpmDetect dlg;
    dlg.enableControls(true);
    QVERIFY(dlg.btnAddFiles->isEnabled());
    dlg.enableControls(false);
    QVERIFY(!dlg.btnAddFiles->isEnabled());
}

void DlgBpmDetectTest::testSetStarted() {
    DlgBpmDetect dlg;
    dlg.setStarted(true);
    QVERIFY(dlg.started());
    dlg.setStarted(false);
    QVERIFY(!dlg.started());
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

QTEST_MAIN(DlgBpmDetectTest)

#include "dlgbpmdetecttest.moc"
