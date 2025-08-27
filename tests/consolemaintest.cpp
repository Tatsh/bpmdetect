// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtTest/QtTest>

#include "consolemain.h"
#include "ffmpegutils.h"
#include "utils.h"

class ConsoleMainTest : public QObject {
    Q_OBJECT
public:
    explicit ConsoleMainTest(QObject *parent = nullptr);
    ~ConsoleMainTest() override;

private Q_SLOTS:
    void testNoArgs();
    void testRemoveBpmTag();
    void testDetection();
    void testDetectUndecodable();
};

ConsoleMainTest::ConsoleMainTest(QObject *parent) : QObject(parent) {
}

ConsoleMainTest::~ConsoleMainTest() {
}

static QString makeTempFileName(const QString &fileName,
                                const QString &prefix = QStringLiteral("/.~")) {
    return QDir::current().absolutePath() + prefix + QFileInfo(fileName).fileName();
}

void ConsoleMainTest::testNoArgs() {
    QCommandLineParser parser;
    auto app = QCoreApplication::instance();
    parseCommandLine(parser, *app);
    QCOMPARE(consoleMain(*app, parser, {}), -1);
}

void ConsoleMainTest::testRemoveBpmTag() {
    auto tempFile = makeTempFileName(QString::fromUtf8(TEST_FILE_140BPM));
    QFile::remove(tempFile);
    QFile::copy(QString::fromUtf8(TEST_FILE_140BPM), tempFile);

    storeBpmInFile(tempFile, QStringLiteral("140.00"));
    auto tags = readTagsFromFile(tempFile);
    QCOMPARE(tags[QStringLiteral("bpm")].toDouble(), 140.0);

    auto tempFileDup = strdup(tempFile.toUtf8().constData());
    const char *argv[] = {"bpmdetect", "--remove", tempFileDup};
    auto argc = 3;

    QCommandLineParser parser;
    QCoreApplication app(argc, const_cast<char **>(argv));
    parseCommandLine(parser, app);
    QVERIFY(parser.isSet(QStringLiteral("remove")));
    auto ret = consoleMain(app, parser, parser.positionalArguments());
    free(tempFileDup);
    QCOMPARE(ret, 0);

    tags = readTagsFromFile(tempFile);
    QCOMPARE(tags[QStringLiteral("bpm")].toDouble(), 0.0);

    QFile::remove(tempFile);
}

void ConsoleMainTest::testDetection() {
    auto tempFile = makeTempFileName(QString::fromUtf8(TEST_FILE_140BPM));
    QFile::remove(tempFile);
    QVERIFY(QFile::copy(QString::fromUtf8(TEST_FILE_140BPM), tempFile));
    auto tags = readTagsFromFile(tempFile);
    QCOMPARE(tags[QStringLiteral("bpm")].toDouble(), 0.0);

    auto tempFile2 = makeTempFileName(QString::fromUtf8(TEST_FILE_140BPM), QStringLiteral("/.~~"));
    QFile::remove(tempFile2);
    QVERIFY(QFile::copy(QString::fromUtf8(TEST_FILE_140BPM), tempFile2));
    storeBpmInFile(tempFile2, QStringLiteral("120.00"));
    tags = readTagsFromFile(tempFile2);
    auto temp2Bpm = tags[QStringLiteral("bpm")].toDouble();
    QVERIFY(temp2Bpm >= 120.0 && temp2Bpm < 120.1);

    auto tempFileDup = strdup(tempFile.toUtf8().constData());
    auto tempFile2Dup = strdup(tempFile2.toUtf8().constData());
    const char *argv[] = {"bpmdetect", "--save", tempFileDup, tempFile2Dup};
    auto argc = 4;

    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());
    QCommandLineParser parser;
    QCoreApplication app(argc, const_cast<char **>(argv));
    parseCommandLine(parser, app);
    QVERIFY(parser.isSet(QStringLiteral("save")));

    auto ret = consoleMain(app, parser, parser.positionalArguments());
    free(tempFileDup);
    free(tempFile2Dup);
    std::cout.rdbuf(old);
    auto output = QString::fromStdString(buffer.str());
    QVERIFY(output.contains(QStringLiteral("/.~140bpm.ogg: 140")));
    QVERIFY(output.contains(QStringLiteral("/.~~140bpm.ogg: 120")));
    QCOMPARE(ret, 0);

    tags = readTagsFromFile(tempFile);
    auto setBpm = tags[QStringLiteral("bpm")].toDouble();
    QVERIFY(setBpm > 140.0 && setBpm < 140.1);

    QFile::remove(tempFile);
    QFile::remove(tempFile2);
}

void ConsoleMainTest::testDetectUndecodable() {
    const char *argv[] = {"bpmdetect", "CMakeLists.txt"};
    auto argc = 2;

    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());

    QCommandLineParser parser;
    QCoreApplication app(argc, const_cast<char **>(argv));
    parseCommandLine(parser, app);
    auto ret = consoleMain(app, parser, parser.positionalArguments());

    std::cout.rdbuf(old);
    auto output = QString::fromStdString(buffer.str());
    qDebug() << output;
    QVERIFY(output.contains(QStringLiteral("File is not decodable, skipping:")));
    QCOMPARE(ret, 0);
}

QTEST_GUILESS_MAIN(ConsoleMainTest)

#include "consolemaintest.moc"
