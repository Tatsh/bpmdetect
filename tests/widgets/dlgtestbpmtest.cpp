#include <QtTest>

#include "widgets/dlgtestbpm.h"

class DlgTestBpmTest : public QObject {
    Q_OBJECT
public:
    explicit DlgTestBpmTest(QObject *parent = nullptr);
    ~DlgTestBpmTest() override;

private Q_SLOTS:
    void testConstructor();
    void testSetTrackPositionLength();
    void testSetCustomPos();
    void testSetNumBeats();
    void testSlotUpdateBpmList();
};

DlgTestBpmTest::DlgTestBpmTest(QObject *parent) : QObject(parent) {
}

DlgTestBpmTest::~DlgTestBpmTest() {
}

void DlgTestBpmTest::testConstructor() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV), 120.0f);
    QVERIFY(dlg.isVisible() || !dlg.isVisible()); // Just ensure construction does not crash
}

void DlgTestBpmTest::testSetTrackPositionLength() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV), 120.0f);
    dlg.setTrackPositionLength(5000);
    QVERIFY(dlg.trackPosition->isEnabled());
}

void DlgTestBpmTest::testSetCustomPos() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV), 120.0f);
    dlg.setCustomPos(1000); // The msec value is not used.
    QCOMPARE(dlg.trackPosition->value(), -1);
}

void DlgTestBpmTest::testSetNumBeats() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV), 120.0f);
    dlg.setNumBeats(QStringLiteral("4"));
    QCOMPARE(dlg.cbNBeats->currentText(), QStringLiteral("4"));
}

void DlgTestBpmTest::testSlotUpdateBpmList() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV), 120.0f);
    dlg.slotUpdateBpmList();
    QVERIFY(!dlg.m_bpmList.isEmpty());
}

QTEST_MAIN(DlgTestBpmTest)

#include "dlgtestbpmtest.moc"
