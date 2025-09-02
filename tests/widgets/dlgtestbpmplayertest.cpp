#include "widgets/dlgtestbpmplayer.h"
#include <QtTest>

class DlgTestBpmPlayerTest : public QObject {
    Q_OBJECT
public:
    explicit DlgTestBpmPlayerTest(QObject *parent = nullptr);
    ~DlgTestBpmPlayerTest() override;

private Q_SLOTS:
    void testDecodeError();
    void testStart();
};

DlgTestBpmPlayerTest::DlgTestBpmPlayerTest(QObject *parent) : QObject(parent) {
}

DlgTestBpmPlayerTest::~DlgTestBpmPlayerTest() {
}

void DlgTestBpmPlayerTest::testDecodeError() {
    auto player = new DlgTestBpmPlayer(QStringLiteral("nonexistent-file.wav"), 1, 140, 0, this);
    player->decodeError(QAudioDecoder::ResourceError);
    QVERIFY(player->error_);
}

void DlgTestBpmPlayerTest::testStart() {
    QEventLoop loop;
    auto player = new DlgTestBpmPlayer(QStringLiteral(TEST_FILE), 1, 140, 0, this);
    connect(player->decoder_, &QAudioDecoder::finished, [&loop, player]() {
        QThread::sleep(3);
        player->update(1, 2000000);
        QThread::sleep(3);
        player->stop();
        QThread::sleep(1);
        loop.quit();
    });
    player->start();
    loop.exec();
    player->terminate();
    QVERIFY(!player->error_);
}

QTEST_MAIN(DlgTestBpmPlayerTest)

#include "dlgtestbpmplayertest.moc"
