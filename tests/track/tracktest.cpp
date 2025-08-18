#include <iostream>

#include <QtTest>

#include "track/track.h"

class DummyTrack : public Track {
public:
    DummyTrack() : Track() {
    }
    void seek(quint64) override {
    }
    quint64 currentPos() const override {
        return currentPos_;
    }
    int readSamples(QSpan<soundtouch::SAMPLETYPE>) override {
        currentPos_ += 1000;
        return 4096;
    }
    void storeBpm(const QString &s) override {
        storeBpmCalled = true;
        storedBpm = s;
    }
    void removeBpm() override {
    }
    void readTags() override {
    }
    void readInfo() override {
        setChannels(2);
        setSampleRate(44100);
        setLength(20000);
        setValid(true);
    }
    bool storeBpmCalled = false;
    QString storedBpm;
    quint64 currentPos_ = 0;
};

class DummyBpmDetector : public AbstractBpmDetector {
    Q_OBJECT
public:
    DummyBpmDetector(QObject *parent = nullptr) : AbstractBpmDetector(parent) {
    }
    void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) const override {
        // Dummy implementation, no actual processing needed for this test
    }
    bpmtype getBpm() const override {
        return 120.0; // Dummy value for testing
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
    void testDetectBpm();
    void testFormatted1();
    void testFormatted2();
    void testFormattedLength();
    void testInitialProgress();
    void testOpen();
    void testPrintBpm();
    void testProgressAfterLengthSet();
    void testProgressBounds();
    void testSampleBits();
    void testSetAndGetArtist();
    void testSetAndGetBpm();
    void testSetAndGetChannels();
    void testSetAndGetFileName();
    void testSetAndGetFormat();
    void testSetAndGetLength();
    void testSetAndGetOpened();
    void testSetAndGetRedetect();
    void testSetAndGetSampleBytes();
    void testSetAndGetSampleRate();
    void testSetAndGetStartEndPos();
    void testSetAndGetTitle();
    void testSetAndGetTrackType();
    void testSetAndGetValid();
    void testSetConsoleProgress();
    void testSetMaximumBpmSwap();
    void testSetMinimumBpmSwap();
    void testSetProgress();
    void testSetSampleBytesAbove4();
    void testStaticBpmLimits();
    void testStop();
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

void TrackTest::testSetAndGetArtist() {
    DummyTrack t;
    t.setArtist(QStringLiteral("Artist"));
    QCOMPARE(t.artist(), QStringLiteral("Artist"));
}

void TrackTest::testSetAndGetTitle() {
    DummyTrack t;
    t.setTitle(QStringLiteral("Title"));
    QCOMPARE(t.title(), QStringLiteral("Title"));
}

void TrackTest::testSetAndGetLength() {
    DummyTrack t;
    t.setLength(123456);
    QCOMPARE(t.length(), static_cast<quint64>(123456));
}

void TrackTest::testSetAndGetChannels() {
    DummyTrack t;
    t.setChannels(2);
    QCOMPARE(t.channels(), 2u);
}

void TrackTest::testSetAndGetSampleBytes() {
    DummyTrack t;
    t.setSampleBytes(2);
    QCOMPARE(t.sampleBytes(), 2u);
}

void TrackTest::testSetAndGetSampleRate() {
    DummyTrack t;
    t.setSampleRate(44100);
    QCOMPARE(t.sampleRate(), 44100u);
}

void TrackTest::testSetAndGetTrackType() {
    DummyTrack t;
    t.setTrackType(Track::Mp3);
    QCOMPARE(t.trackType(), Track::Mp3);
}

void TrackTest::testSetAndGetFileName() {
    DummyTrack t;
    t.setFileName(QStringLiteral("file.wav"), true);
    QCOMPARE(t.fileName(), QStringLiteral("file.wav"));
}

void TrackTest::testSetAndGetValid() {
    DummyTrack t;
    t.setValid(true);
    QVERIFY(t.isValid());
    t.setValid(false);
    QVERIFY(!t.isValid());
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

void TrackTest::testSetAndGetStartEndPos() {
    DummyTrack t;
    t.setLength(1000);
    t.setEndPos(900);
    t.setStartPos(100);
    QCOMPARE(t.startPos(), static_cast<quint64>(100));
    QCOMPARE(t.endPos(), static_cast<quint64>(900));

    t.setEndPos(1100); // Should not change startPos
    QCOMPARE(t.endPos(), static_cast<quint64>(900));

    t.setStartPos(1100); // Should not change endPos
    QCOMPARE(t.startPos(), static_cast<quint64>(100));

    t.setEndPos(50);
    QCOMPARE(t.startPos(), static_cast<quint64>(50)); // Should swap start and end
    QCOMPARE(t.endPos(), static_cast<quint64>(100));

    t.setStartPos(150);
    QCOMPARE(t.startPos(), static_cast<quint64>(100)); // Should swap back
    QCOMPARE(t.endPos(), static_cast<quint64>(150));
}

void TrackTest::testStaticBpmLimits() {
    Track::setMinimumBpm(90.);
    Track::setMaximumBpm(180.);
    QCOMPARE(Track::minimumBpm(), 90.);
    QCOMPARE(Track::maximumBpm(), 180.);
}

void TrackTest::testInitialProgress() {
    DummyTrack t;
    QCOMPARE(t.progress(), 0.0);
}

void TrackTest::testSetProgress() {
    DummyTrack t;
    t.setLength(1000);
    t.setProgress(0.5);
    QCOMPARE(t.progress(), 0.5);
    QCOMPARE(t.startPos(), static_cast<quint64>(0));
    QCOMPARE(t.endPos(), static_cast<quint64>(0));
}

void TrackTest::testProgressAfterLengthSet() {
    DummyTrack t;
    t.setLength(2000);
    t.setProgress(0.25);
    QCOMPARE(t.progress(), 0.25);
}

void TrackTest::testProgressBounds() {
    DummyTrack t;
    t.setLength(1000);
    t.setProgress(-0.1);
    QVERIFY(t.progress() >= 0.0);
    t.setProgress(1.1);
    QVERIFY(t.progress() == 1.1);
}

void TrackTest::testSetConsoleProgress() {
    DummyTrack t;
    t.setConsoleProgress(true);
    QVERIFY(t.m_bConProgress);
}

void TrackTest::testFormattedLength() {
    DummyTrack t;
    t.setLength(123456);
    QCOMPARE(t.formattedLength(), QStringLiteral("02:03"));
}

void TrackTest::testClearBpm() {
    DummyTrack t;
    t.setBpm(120.0);
    QVERIFY(t.m_dBpm == 120.0);
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

void TrackTest::testStop() {
    DummyTrack t;
    t.m_bStop = false;
    t.stop();
    QVERIFY(t.m_bStop);
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

void TrackTest::testSetSampleBytesAbove4() {
    DummyTrack t;
    t.setSampleBytes(5); // Invalid.
    QCOMPARE(t.sampleBytes(), 0);

    DummyTrack t2;
    t2.setSampleBytes(16); // Valid.
    QCOMPARE(t2.sampleBytes(), 2);

    DummyTrack t3;
    t3.setSampleBytes(128); // Valid but too large.
    QCOMPARE(t3.sampleBytes(), 0);
}

void TrackTest::testSampleBits() {
    DummyTrack t;
    t.setSampleBytes(4);
    QCOMPARE(t.sampleBits(), 32);
}

void TrackTest::testStoreBpm() {
    DummyTrack t;
    t.setBpm(123.45);
    t.setFormat(QStringLiteral("00000"));
    t.saveBpm();
    QVERIFY(t.storeBpmCalled);
    QCOMPARE(t.storedBpm, QStringLiteral("00123"));
}

void TrackTest::testDetectBpm() {
    DummyTrack t;
    QCOMPARE(t.bpm(), 0.0);
    t.setDetector(new DummyBpmDetector(this));
    QCOMPARE(t.detectBpm(), 0.0); // Invalid state.

    t.setFileName(QStringLiteral("dummy.wav"), true);
    t.setLength(2000);
    t.setEndPos(1000);
    QVERIFY(t.isValid());
    QVERIFY(t.m_iEndPos == 1000);
    QCOMPARE(t.detectBpm(), 120.0);

    t.m_bRedetect = false;
    QCOMPARE(t.detectBpm(), 120.0);

    t.m_bRedetect = true;
    t.setSampleRate(0);
    QCOMPARE(t.detectBpm(), 120.0);
}

QTEST_MAIN(TrackTest)

#include "tracktest.moc"
