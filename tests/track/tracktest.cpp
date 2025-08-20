#include <iostream>

#include <QtTest>

#include "track/track.h"

struct DummyTrack : public Track {
    DummyTrack() : Track(QStringLiteral(""), false, this) {
    }
    ~DummyTrack() override {
    }
    bpmtype detectBpm() {
        return 0;
    }
    bool isOpened() const {
        return opened_;
    }
    void setOpened(bool opened) {
        opened_ = opened;
    }
    bool storeBpmCalled_ = false;
    QString storedBpm_;
    quint64 currentPos_ = 0;
    bool opened_ = false;
};

struct DummyBpmDetector : public AbstractBpmDetector {
    Q_OBJECT
    DummyBpmDetector(QObject *parent = nullptr) : AbstractBpmDetector(parent) {
    }
    void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) const override {
    }
    bpmtype getBpm() const override {
        return 120.0;
    }
    void reset(int channels, int sampleRate) override {
        Q_UNUSED(channels)
        Q_UNUSED(sampleRate)
    }
};

class TrackTest : public QObject {
    Q_OBJECT
public:
    explicit TrackTest(QObject *parent = nullptr);
    ~TrackTest() override;

private Q_SLOTS:
    void testClearBpm();
    void testCorrectBpm();
    void testFormatted1();
    void testFormatted2();
    void testOpen();
    void testPrintBpm();
    void testSetAndGetBpm();
    void testSetAndGetFileName();
    void testSetAndGetFormat();
    void testSetAndGetOpened();
    void testSetAndGetRedetect();
    void testSetMaximumBpmSwap();
    void testSetMinimumBpmSwap();
    void testStaticBpmLimits();
    void testStoreBpm();
};

TrackTest::TrackTest(QObject *parent) : QObject(parent) {
}

TrackTest::~TrackTest() {
}

void TrackTest::testSetAndGetBpm() {
    DummyTrack t;
    t.setBpm(123.45);
    QCOMPARE(t.bpm(), 123.45);
}

void TrackTest::testSetAndGetFormat() {
    DummyTrack t;
    t.setFormat(QStringLiteral("0.0"));
    QCOMPARE(t.format(), QStringLiteral("0.0"));
}

void TrackTest::testSetAndGetFileName() {
    DummyTrack t;
    t.setFileName(QStringLiteral("file.wav"), true);
    QCOMPARE(t.fileName(), QStringLiteral("file.wav"));
}

void TrackTest::testSetAndGetOpened() {
    DummyTrack t;
    t.setOpened(true);
    QVERIFY(t.isOpened());
    t.setOpened(false);
    QVERIFY(!t.isOpened());
}

void TrackTest::testSetAndGetRedetect() {
    DummyTrack t;
    t.setRedetect(true);
    QVERIFY(t.redetect());
    t.setRedetect(false);
    QVERIFY(!t.redetect());
}

void TrackTest::testStaticBpmLimits() {
    Track::setMinimumBpm(90.);
    Track::setMaximumBpm(180.);
    QCOMPARE(Track::minimumBpm(), 90.);
    QCOMPARE(Track::maximumBpm(), 180.);
}

void TrackTest::testClearBpm() {
    DummyTrack t;
    t.setBpm(120.0);
    QVERIFY(t.dBpm_ == 120.0);
    t.clearBpm();
    QCOMPARE(t.bpm(), 0.0);
}

void TrackTest::testCorrectBpm() {
    DummyTrack t;
    t.setMinimumBpm(60.0);
    t.setMaximumBpm(180.0);

    QCOMPARE(t.correctBpm(50.0), 100.0);
    QCOMPARE(t.correctBpm(200.0), 100.0);
    QCOMPARE(t.correctBpm(120.0), 120.0);
    QCOMPARE(t.correctBpm(-1), 0.0);
}

void TrackTest::testSetMinimumBpmSwap() {
    Track::setMaximumBpm(100.0);
    Track::setMinimumBpm(200.0); // min > max, should swap
    QCOMPARE(Track::minimumBpm(), 100.0);
    QCOMPARE(Track::maximumBpm(), 200.0);
}

void TrackTest::testSetMaximumBpmSwap() {
    Track::setMinimumBpm(100.0);
    Track::setMaximumBpm(50.0); // max < min, should swap
    QCOMPARE(Track::minimumBpm(), 50.0);
    QCOMPARE(Track::maximumBpm(), 100.0);
}

void TrackTest::testPrintBpm() {
    DummyTrack t;
    t.setFileName(QStringLiteral("test.wav"), false);
    t.setBpm(123.45);
    t.setFormat(QStringLiteral("0.00"));
    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());
    t.printBpm();
    std::cout.rdbuf(old);
    QString output = QString::fromStdString(buffer.str());
    QVERIFY(output.contains(QStringLiteral("test.wav: 123.45 BPM")));
}

void TrackTest::testOpen() {
    DummyTrack t;
    t.open();
    QVERIFY(t.isOpened());
}

void TrackTest::testFormatted1() {
    DummyTrack t;
    t.setBpm(123.456);
    t.setFormat(QStringLiteral("0.00"));
    QString formatted = t.formatted();
    QCOMPARE(formatted, QStringLiteral("123.46"));
    // Test with a different format.
    t.setFormat(QStringLiteral("0.000")); // Defaults to 0.00.
    formatted = t.formatted();
    QCOMPARE(formatted, QStringLiteral("123.46"));
}

void TrackTest::testFormatted2() {
    DummyTrack t;
    t.setBpm(123.456);
    t.setFormat(QStringLiteral("0.00"));
    QString formatted = t.formatted(QStringLiteral("0.00"));
    QCOMPARE(formatted, QStringLiteral("123.46"));
    // Test with a different format.
    t.setFormat(QStringLiteral("0.0"));
    formatted = t.formatted(QStringLiteral("0.000")); // Defaults to 0.00.
    QCOMPARE(formatted, QStringLiteral("123.46"));
}

void TrackTest::testStoreBpm() {
    DummyTrack t;
    t.setBpm(123.45);
    t.setFormat(QStringLiteral("00000"));
    t.saveBpm();
    QVERIFY(t.storeBpmCalled_);
    QCOMPARE(t.storedBpm_, QStringLiteral("00123"));
}

QTEST_MAIN(TrackTest)

#include "tracktest.moc"
