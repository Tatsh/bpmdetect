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
    auto player = new DlgTestBpmPlayer(
        QStringLiteral("nonexistent-file.wav"), 1, 140, new QAudioDecoder(this), 0, this);
    player->decodeError(QAudioDecoder::ResourceError);
    QVERIFY(player->error);
}

void DlgTestBpmPlayerTest::testStart() {
    QEventLoop loop;
    auto decoder = new QAudioDecoder(this);
    auto player = new DlgTestBpmPlayer(QStringLiteral(TEST_FILE), 1, 140, decoder, 0, this);
    connect(decoder, &QAudioDecoder::finished, [&loop, player]() {
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
    QVERIFY(!player->error);
}

QTEST_MAIN(DlgTestBpmPlayerTest)

#include "dlgtestbpmplayertest.moc"
