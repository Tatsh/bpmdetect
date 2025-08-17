#include "widgets/dlgtestbpmplayer.h"
#include <QtTest>

class DlgTestBpmPlayerTest : public QObject {
    Q_OBJECT
public:
    explicit DlgTestBpmPlayerTest(QObject *parent = nullptr);
    ~DlgTestBpmPlayerTest() override;

private Q_SLOTS:
    void testConstructor();
    void testLengthUs();
    void testUpdate();
    void testStop();
};

DlgTestBpmPlayerTest::DlgTestBpmPlayerTest(QObject *parent) : QObject(parent) {
}

DlgTestBpmPlayerTest::~DlgTestBpmPlayerTest() {
}

void DlgTestBpmPlayerTest::testConstructor() {
    DlgTestBpmPlayer player(QStringLiteral(NOISE_WAV), 4, 120, 0, this);
    QVERIFY(!player.isRunning());
    QCOMPARE(player.lengthUs(), 0);
}

void DlgTestBpmPlayerTest::testLengthUs() {
    DlgTestBpmPlayer player(QStringLiteral(NOISE_WAV), 4, 120, 0, this);
    QCOMPARE(player.lengthUs(), 0);
}

void DlgTestBpmPlayerTest::testUpdate() {
    DlgTestBpmPlayer player(QStringLiteral(NOISE_WAV), 4, 120, 0, this);
    player.update(8, 1000000);
    // Verify something here.
}

void DlgTestBpmPlayerTest::testStop() {
    DlgTestBpmPlayer player(QStringLiteral(NOISE_WAV), 4, 120, 0);
    player.stop();
    // Verify something here.
}

QTEST_MAIN(DlgTestBpmPlayerTest)

#include "dlgtestbpmplayertest.moc"
