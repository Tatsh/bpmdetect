
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkInterface>
#include <QtTest/QTest>

#include "core/metronome.h"

class MetronomeTest : public QObject {
    Q_OBJECT

public:
    MetronomeTest(QObject *parent = nullptr) {
        Q_UNUSED(parent);
    };
    ~MetronomeTest() override {};

private Q_SLOTS:
    void testSetInterval();
    void testSetIntervalBelow100Ms();
    void testSetBPM();
    void testProgress();
    void testProgressBeforeStarted();
};

void MetronomeTest::testSetInterval() {
    Metronome metronome;
    metronome.setInterval(500);
    QVERIFY(metronome.m_interval == 500);
}

void MetronomeTest::testSetIntervalBelow100Ms() {
    Metronome metronome;
    metronome.setInterval(50);
    QVERIFY(metronome.m_interval == 100);
}

void MetronomeTest::testSetBPM() {
    Metronome metronome;
    metronome.setBPM(120.0);
    QVERIFY(metronome.m_interval == 500);
}

void MetronomeTest::testProgress() {
    Metronome metronome;
    metronome.setInterval(1000);
    metronome.start();
    metronome.setSync(0);
    QTest::qSleep(500);
    unsigned long progress = metronome.progress();
    QVERIFY(progress >= 0 && progress < 1000);
    float percent = metronome.progressPercent();
    QVERIFY(percent >= 0.0 && percent < 100.0);
}

void MetronomeTest::testProgressBeforeStarted() {
    Metronome metronome;
    metronome.setInterval(1000);
    unsigned long progress = metronome.progress();
    QVERIFY(progress == 0);
    float percent = metronome.progressPercent();
    QVERIFY(percent == 0.0);
}

QTEST_MAIN(MetronomeTest)

#include "metronometest.moc"
