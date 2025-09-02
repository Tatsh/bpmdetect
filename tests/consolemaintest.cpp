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

void ConsoleMainTest::testNoArgs() {
    QCommandLineParser parser;
    auto app = QCoreApplication::instance();
    parseCommandLine(parser, *app);
    QCOMPARE(consoleMain(*app, parser, {}), -1);
}

static void copyToTempFile(const QString &source, QTemporaryFile &tempFile) {
    QFile sourceFile(source);
    QFileInfo fi(source);
    tempFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/XXXXXX.") + fi.suffix());
    QVERIFY(sourceFile.open(QIODevice::ReadOnly));
    QVERIFY(tempFile.open());
    tempFile.write(sourceFile.readAll());
    sourceFile.close();
    tempFile.close();

    qDebug() << "Copied" << source << "to" << tempFile.fileName();
}

void ConsoleMainTest::testRemoveBpmTag() {
    QTemporaryFile tempFile;
    copyToTempFile(QString::fromUtf8(TEST_FILE_140BPM), tempFile);
    qDebug() << "Temp file:" << tempFile.fileName();

    storeBpmInFile(tempFile.fileName(), QStringLiteral("140.00"));
    auto tags = readTagsFromFile(tempFile.fileName());
    QCOMPARE(tags[QStringLiteral("bpm")].toDouble(), 140.0);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    auto tempFileDup = strdup(tempFile.fileName().toUtf8().constData());
#pragma clang diagnostic pop
    const char *argv[] = {"bpmdetect", "--remove", tempFileDup};
    auto argc = 3;

    QCommandLineParser parser;
    QCoreApplication app(argc, const_cast<char **>(argv));
    parseCommandLine(parser, app);
    QVERIFY(parser.isSet(QStringLiteral("remove")));
    auto ret = consoleMain(app, parser, parser.positionalArguments());
    free(tempFileDup);
    QCOMPARE(ret, 0);

    tags = readTagsFromFile(tempFile.fileName());
    QCOMPARE(tags[QStringLiteral("bpm")].toDouble(), 0.0);
}

void ConsoleMainTest::testDetection() {
    QTemporaryFile tempFile, tempFile2;
    tempFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/XXXXXX.ogg"));
    copyToTempFile(QString::fromUtf8(TEST_FILE_140BPM), tempFile);
    auto tags = readTagsFromFile(tempFile.fileName());
    QCOMPARE(tags[QStringLiteral("bpm")].toDouble(), 0.0);

    copyToTempFile(QString::fromUtf8(TEST_FILE_140BPM), tempFile2);
    storeBpmInFile(tempFile2.fileName(), QStringLiteral("120.00"));
    tags = readTagsFromFile(tempFile2.fileName());
    auto temp2Bpm = tags[QStringLiteral("bpm")].toDouble();
    QVERIFY(temp2Bpm >= 120.0 && temp2Bpm < 120.1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    auto tempFileDup = strdup(tempFile.fileName().toUtf8().constData());
    auto tempFile2Dup = strdup(tempFile2.fileName().toUtf8().constData());
#pragma clang diagnostic pop
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
    QVERIFY(output.contains(QStringLiteral(".ogg: 140")));
    QVERIFY(output.contains(QStringLiteral(".ogg: 120")));
    QCOMPARE(ret, 0);

    tags = readTagsFromFile(tempFile.fileName());
    auto setBpm = tags[QStringLiteral("bpm")].toDouble();
    QVERIFY(setBpm > 140.0 && setBpm < 140.1);
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
