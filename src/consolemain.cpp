// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QAtomicInt>
#include <QtCore/QEventLoop>

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
            Track track(file, false);
            track.clearBpm();
        }
        return 0;
    }
    QEventLoop loop;
    QAtomicInt pendingTracks(files.size());
    for (const auto &file : files) {
        auto track = new Track(file, false, &app);
        track->setRedetect(detect);
        track->setFormat(format);
        track->setDetector(new SoundTouchBpmDetector(track));
        // track.setConsoleProgress(consoleProgress);
        QObject::connect(track, &Track::hasBpm, [track, &pendingTracks, &loop, &save](bpmtype bpm) {
            track->printBpm();
            if (save) {
                track->saveBpm();
            }
            if (--pendingTracks == 0) {
                loop.quit();
            }
            track->deleteLater();
        });
        track->detectBpm();
    }
    loop.exec();
    return 0;
}
