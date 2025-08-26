#include <iostream>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtMultimedia/QAudioDecoder>
#include <QtTest>

#include "track/track.h"

struct DummyTrack : public Track {
    DummyTrack() : Track() {
    }
    DummyTrack(const QString &fileName) : Track(fileName) {
    }
    ~DummyTrack() override {
    }
};

class DummyBpmDetector : public AbstractBpmDetector {
    Q_OBJECT
public:
    DummyBpmDetector(QObject *parent = nullptr) : AbstractBpmDetector(parent) {
    }
    void inputSamples(const soundtouch::SAMPLETYPE *samples, int numSamples) override {
    }
    bpmtype getBpm() const override {
        return 120.0;
    }
    void reset() override {
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
    void testPrintBpm();
    void testSetAndGetBpm();
    void testSetAndGetFileName();
    void testSetAndGetFormat();
    void testSetMaximumBpmSwap();
    void testSetMinimumBpmSwap();
    void testStaticBpmLimits();
    void testValidFile();
    void testSetBpmTag();
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
    DummyTrack t(QStringLiteral("file.wav"));
    QCOMPARE(t.fileName(), QStringLiteral("file.wav"));
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
    DummyTrack t(QStringLiteral("test.wav"));
    t.setBpm(123.45);
    t.setFormat(QStringLiteral("0.00"));
    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());
    t.printBpm();
    std::cout.rdbuf(old);
    QString output = QString::fromStdString(buffer.str());
    QVERIFY(output.contains(QStringLiteral("test.wav: 123.45 BPM")));
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

void TrackTest::testValidFile() {
    Track t(QString::fromUtf8(TEST_FILE_5S_SILENT), new QAudioDecoder(this));
    t.setDetector(new DummyBpmDetector(this));
    QCOMPARE(t.artist(), QStringLiteral("Artist"));
    QCOMPARE(t.title(), QStringLiteral("Title"));
    QVERIFY(t.length() >= 5000);
    QVERIFY(!t.hasValidBpm());
    QCOMPARE(t.detectBpm(), Track::Detecting);
    QVERIFY(t.isValid());
}

void TrackTest::testSetBpmTag() {
    auto sourceName = QString::fromUtf8(TEST_FILE_5S_SILENT);
    auto tempFile =
        QDir::currentPath() + QStringLiteral("/.test-output-") + QFileInfo(sourceName).fileName();
    QFile::remove(tempFile);
    QVERIFY(QFile::copy(sourceName, tempFile));

    Track t(tempFile, new QAudioDecoder(this));
    t.setDetector(new DummyBpmDetector(this));
    QCOMPARE(t.artist(), QStringLiteral("Artist"));
    QCOMPARE(t.title(), QStringLiteral("Title"));
    QVERIFY(t.length() >= 5000);
    QVERIFY(!t.hasValidBpm());
    t.dBpm_ = 120.0;
    t.saveBpm();
    QVERIFY(t.hasValidBpm());

    Track t2(tempFile, new QAudioDecoder(this));
    t2.setDetector(new DummyBpmDetector(this));
    QCOMPARE(t2.artist(), QStringLiteral("Artist"));
    QCOMPARE(t2.title(), QStringLiteral("Title"));
    QVERIFY(t2.length() >= 5000);
    QVERIFY(t2.hasValidBpm());
    QCOMPARE(t2.bpm(), 120.0);
    t2.clearBpm();

    Track t3(tempFile, new QAudioDecoder(this));
    t3.setDetector(new DummyBpmDetector(this));
    QCOMPARE(t3.artist(), QStringLiteral("Artist"));
    QCOMPARE(t3.title(), QStringLiteral("Title"));
    QVERIFY(t3.length() >= 5000);
    QVERIFY(!t3.hasValidBpm());

    QFile::remove(tempFile);
}

QTEST_MAIN(TrackTest)

#include "tracktest.moc"
