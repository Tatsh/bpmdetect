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
    void testSetPosFromButton();
};

class NukedTestBpmPlayer : public DlgTestBpmPlayer {
public:
    NukedTestBpmPlayer(const QString &file,
                       unsigned int nBeats_,
                       unsigned int bpm_,
                       qint64 posUS_ = 0,
                       QObject *parent = nullptr)
        : DlgTestBpmPlayer(file, nBeats_, bpm_, new QAudioDecoder(), posUS_, parent) {
    }
    ~NukedTestBpmPlayer() override {
    }
};

DlgTestBpmTest::DlgTestBpmTest(QObject *parent) : QObject(parent) {
}

DlgTestBpmTest::~DlgTestBpmTest() {
}

void DlgTestBpmTest::testConstructor() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV),
                   120.0f,
                   new NukedTestBpmPlayer(QStringLiteral(NOISE_WAV), 4, 120, 0, this));
    QVERIFY(dlg.isVisible() || !dlg.isVisible()); // Just ensure construction does not crash
}

void DlgTestBpmTest::testSetTrackPositionLength() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV),
                   120.0f,
                   new NukedTestBpmPlayer(QStringLiteral(NOISE_WAV), 4, 120, 0, this));
    dlg.setTrackPositionLength(5000);
    QVERIFY(dlg.trackPosition->isEnabled());
}

void DlgTestBpmTest::testSetCustomPos() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV),
                   120.0f,
                   new NukedTestBpmPlayer(QStringLiteral(NOISE_WAV), 4, 120, 0, this));
    dlg.setCustomPos(1000); // The msec value is not used.
    QCOMPARE(dlg.trackPosition->value(), -1);
}

void DlgTestBpmTest::testSetNumBeats() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV),
                   120.0f,
                   new NukedTestBpmPlayer(QStringLiteral(NOISE_WAV), 4, 120, 0, this));
    dlg.setNumBeats(QStringLiteral("4"));
    QCOMPARE(dlg.cbNBeats->currentText(), QStringLiteral("4"));

    dlg.setNumBeats(QStringLiteral("0")); // Invalid, should not change.
    QCOMPARE(dlg.cbNBeats->currentText(), QStringLiteral("4"));

    dlg.setNumBeats(QStringLiteral("not a number"));
    QCOMPARE(dlg.cbNBeats->currentText(), QStringLiteral("4"));
}

void DlgTestBpmTest::testSlotUpdateBpmList() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV),
                   60.25,
                   new NukedTestBpmPlayer(QStringLiteral(NOISE_WAV), 4, 60.25, 0, this));
    dlg.slotUpdateBpmList();
    QVERIFY(!dlg.m_bpmList.isEmpty());
}

void DlgTestBpmTest::testSetPosFromButton() {
    DlgTestBpm dlg(QStringLiteral(NOISE_WAV),
                   120.0f,
                   new NukedTestBpmPlayer(QStringLiteral(NOISE_WAV), 4, 120, 0, this));
    dlg.setPosFromButton(1);
    QCOMPARE(dlg.trackPosition->value(), 20);
    dlg.setPosFromButton(2);
    QCOMPARE(dlg.trackPosition->value(), 40);
}

QTEST_MAIN(DlgTestBpmTest)

#include "dlgtestbpmtest.moc"
