#include "utils.h"

double stringToBpm(const QString &sBpm) {
    auto bpm = sBpm.toDouble();
    while (bpm > 300)
        bpm = bpm / 10;
    return bpm;
}

QString bpmToString(bpmtype dBpm, const QString &format) {
    static const auto zero = QChar::fromLatin1('0');
    if (format == QStringLiteral("0.0")) {
        return QString::number(dBpm, 'f', 1);
    } else if (format == QStringLiteral("0")) {
        return QString::number(dBpm, 'd', 0);
    } else if (format == QStringLiteral("000.00")) {
        return QString::number(dBpm, 'f', 2).rightJustified(6, zero);
    } else if (format == QStringLiteral("000.0")) {
        return QString::number(dBpm, 'f', 1).rightJustified(5, zero);
    } else if (format == QStringLiteral("000")) {
        return QString::number(dBpm, 'd', 0).rightJustified(3, zero);
    } else if (format == QStringLiteral("00000")) {
        return QString::number(dBpm, 'd', 0).rightJustified(5, zero);
    }
    // all other formats are converted to "0.00"
    return QString::number(dBpm, 'f', 2);
}

void parseCommandLine(QCommandLineParser &parser, const QCoreApplication &app) {
    parser.setApplicationDescription(
        QStringLiteral("BPM Detect - automatic BPM detection utility"));

    QCommandLineOption consoleOpt({QStringLiteral("c"), QStringLiteral("console")},
                                  QStringLiteral("Run in console mode."));
#ifdef NO_GUI
    consoleOpt.setFlags(QCommandLineOption::HiddenFromHelp);
#endif
    QCommandLineOption saveOpt({QStringLiteral("s"), QStringLiteral("save")},
                               QCoreApplication::translate("main", "Save BPMs to tags."));
    QCommandLineOption detectOpt({QStringLiteral("d"), QStringLiteral("detect")},
                                 QCoreApplication::translate("main", "Redetect BPMs."));
    QCommandLineOption removeOpt(
        {QStringLiteral("r"), QStringLiteral("remove")},
        QCoreApplication::translate("main", "Remove BPM tags and do not perform detection."));
    QCommandLineOption noProgressOpt(
        {QStringLiteral("p"), QStringLiteral("no-progress")},
        QCoreApplication::translate("main", "Disable progress display."));
    QCommandLineOption minOpt({QStringLiteral("n"), QStringLiteral("min")},
                              QCoreApplication::translate("main", "Minimum BPM."),
                              QStringLiteral("value"));
    QCommandLineOption maxOpt({QStringLiteral("x"), QStringLiteral("max")},
                              QCoreApplication::translate("main", "Maximum BPM."),
                              QStringLiteral("value"));
    QCommandLineOption limitOpt(
        {QStringLiteral("l"), QStringLiteral("limit")},
        QCoreApplication::translate("main", "Do not allow a BPM above the range."));
    QCommandLineOption formatOpt({QStringLiteral("f"), QStringLiteral("format")},
                                 QCoreApplication::translate("main", "Set BPM format."),
                                 QStringLiteral("format"),
                                 QStringLiteral("0.00"));

    parser.addOption(consoleOpt);
    parser.addOption(detectOpt);
    parser.addOption(formatOpt);
    parser.addOption(limitOpt);
    parser.addOption(maxOpt);
    parser.addOption(minOpt);
    parser.addOption(noProgressOpt);
    parser.addOption(removeOpt);
    parser.addOption(saveOpt);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("files"),
                                 QCoreApplication::translate("main", "Files to process."));
    parser.process(app);
}
