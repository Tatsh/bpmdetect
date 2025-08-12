// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCommandLineParser>
#ifndef NO_GUI
#include <QApplication>
#else
#include <QCoreApplication>
#endif

#include "track/trackproxy.h"

#ifndef NO_GUI
#include "widgets/dlgbpmdetect.h"
#endif

int main(int argc, char *argv[]) {
    bool isConsoleMode = true;
#ifndef NO_GUI
    QApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif

    QCoreApplication::setApplicationName(QStringLiteral("bpmdetect"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.7.2"));

    QCommandLineParser parser;
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
    QCommandLineOption noprogressOpt(
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
    parser.addOption(noprogressOpt);
    parser.addOption(removeOpt);
    parser.addOption(saveOpt);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("files"),
                                 QCoreApplication::translate("main", "Files to process."));
    parser.process(app);
#ifndef NO_GUI
    isConsoleMode = parser.isSet(consoleOpt);
#endif

    if (parser.isSet(minOpt)) {
        Track::setMinBPM(parser.value(minOpt).toDouble());
        qDebug() << "Min BPM set to" << Track::getMinBPM();
    }
    if (parser.isSet(maxOpt)) {
        Track::setMaxBPM(parser.value(maxOpt).toDouble());
        qDebug() << "Max BPM set to" << Track::getMaxBPM();
    }
    Track::setLimit(parser.isSet(limitOpt));
    auto files = parser.positionalArguments();

    if (isConsoleMode) {
        if (files.isEmpty()) {
            parser.showHelp(0);
        }
        for (const auto &file : files) {
            TrackProxy track(file);
            if (!parser.isSet(removeOpt)) {
                track.enableConsoleProgress(!parser.isSet(noprogressOpt));
                track.setRedetect(parser.isSet(detectOpt));
                track.setFormat(parser.value(formatOpt));
                track.detectBPM();
                track.printBPM();
                if (parser.isSet(saveOpt)) {
                    track.saveBPM();
                }
            } else {
                track.clearBPM();
            }
        }
        return 0;
    }

#ifndef NO_GUI
    if (!isConsoleMode) {
        DlgBPMDetect mainWin;
        mainWin.show();
        mainWin.slotAddFiles(files);
        app.exec();
    }
#endif
    return 0;
}
