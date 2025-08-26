// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QStringList>
#include <QtTest/QtTest>

#include "consolemain.h"
#include "utils.h"

class ConsoleMainTest : public QObject {
    Q_OBJECT
public:
    explicit ConsoleMainTest(QObject *parent = nullptr);
    ~ConsoleMainTest() override;

private Q_SLOTS:
    void testNoArgs();
    // void testRemoveBpmTag();
    // void testDetectUndecodable();
    // void testDetectHasValidNoRedetect();
    // void testDetectBpmConsoleProgress();
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

QTEST_MAIN(ConsoleMainTest)

#include "consolemaintest.moc"
