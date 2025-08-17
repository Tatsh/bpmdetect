#include <QtTest>

#include "utils.h"

class UtilsTest : public QObject {
    Q_OBJECT
public:
    explicit UtilsTest(QObject *parent = nullptr);
    ~UtilsTest() override;
private Q_SLOTS:
    void testBpmToString();
    void testBpmToString_data();
    void testStringToBpm();
    void testStringToBpm_data();
    void testParseCommandLine();
};

UtilsTest::UtilsTest(QObject *parent) : QObject(parent) {
}

UtilsTest::~UtilsTest() {
}

void UtilsTest::testStringToBpm_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<double>("expected");

    QTest::newRow("normal") << "120" << 120.0;
    QTest::newRow("high value") << "1200" << 120.0;
    QTest::newRow("decimal") << "128.5" << 128.5;
    QTest::newRow("zero") << "0" << 0.0;
    QTest::newRow("empty") << "" << 0.0;
}

void UtilsTest::testStringToBpm() {
    QFETCH(QString, input);
    QFETCH(double, expected);
    QCOMPARE(stringToBpm(input), expected);
}

void UtilsTest::testBpmToString_data() {
    QTest::addColumn<double>("input");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QString>("expected");

    QTest::newRow("0.0 format") << 128.5 << "0.0" << "128.5";
    QTest::newRow("0 format") << 128.5 << "0" << "129";
    QTest::newRow("000.00 format") << 128.5 << "000.00" << "128.50";
    QTest::newRow("000.0 format") << 8.5 << "000.0" << "008.5";
    QTest::newRow("000 format") << 8.5 << "000" << "009";
    QTest::newRow("00000 format") << 8.5 << "00000" << "00009";
    QTest::newRow("other format") << 128.5 << "other" << "128.50";
}

void UtilsTest::testBpmToString() {
    QFETCH(double, input);
    QFETCH(QString, format);
    QFETCH(QString, expected);
    QCOMPARE(bpmToString(input, format), expected);
}

void UtilsTest::testParseCommandLine() {
    int argc = 4;
    const char *argv[] = {"bpmdetect", "-s", "-n", "100"};
    QCoreApplication app(argc, const_cast<char **>(argv));
    QCommandLineParser parser;
    parseCommandLine(parser, app);
    QVERIFY(parser.isSet(QStringLiteral("save")));
    QCOMPARE(parser.value(QStringLiteral("min")), QStringLiteral("100"));
}

QTEST_MAIN(UtilsTest)

#include "utilstest.moc"
