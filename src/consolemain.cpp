// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QEventLoop>
#include <QtMultimedia/QAudioDecoder>

#include "consolemain.h"
#include "debug.h"
#include "track/track.h"

int consoleMain(QCoreApplication &app, QCommandLineParser &parser, const QStringList &files) {
    auto remove = parser.isSet(QStringLiteral("remove"));
    auto consoleProgress = !parser.isSet(QStringLiteral("no-progress"));
    auto detect = parser.isSet(QStringLiteral("detect"));
    auto format = parser.value(QStringLiteral("format"));
    auto save = parser.isSet(QStringLiteral("save"));
    if (files.isEmpty()) {
        parser.showHelp(1);
    }
    if (remove) {
        for (const auto &file : files) {
            Track(file).clearBpm();
        }
        return 0;
    }

    for (const auto &file : files) {
        QEventLoop loop;
        Track track(file, new QAudioDecoder(&app));
        if (track.hasValidBpm() && !detect) {
            track.printBpm();
            track.deleteLater();
            continue;
        }
        track.setFormat(format);
        track.setDetector(new SoundTouchBpmDetector());
        // track.setConsoleProgress(consoleProgress);
        QObject::connect(&track, &Track::hasBpm, [&track, &loop, &save](bpmtype bpm) {
            track.printBpm();
            if (save) {
                track.saveBpm();
            }
            loop.quit();
            track.deleteLater();
        });
        track.detectBpm();
        loop.exec();
    }
    return 0;
}
