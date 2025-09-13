// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NO_GUI
#include <QtWidgets/QApplication>
#else
#include <QtCore/QCoreApplication>
#endif

#include "consolemain.h"
#include "guimain.h"
#include "track/track.h"
#include "utils.h"
#ifndef NO_GUI
#include "widgets/dlgbpmdetect.h"
#endif

int main(int argc, char *argv[]) {
#ifndef NO_GUI
    QApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif
    QCoreApplication::setApplicationName(QStringLiteral("bpmdetect"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.8.8"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("tat.sh"));
    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCommandLineParser parser;
    parseCommandLine(parser, app);
    if (parser.isSet(QStringLiteral("min"))) {
        Track::setMinimumBpm(parser.value(QStringLiteral("min")).toDouble());
    }
    if (parser.isSet(QStringLiteral("max"))) {
        Track::setMaximumBpm(parser.value(QStringLiteral("max")).toDouble());
    }
#ifdef NO_GUI
    return consoleMain(parser, parser.positionalArguments());
#else
    if (parser.isSet(QStringLiteral("console"))) {
        return consoleMain(app, parser, parser.positionalArguments());
    }
    return guiMain(app, parser.positionalArguments());
#endif
}
