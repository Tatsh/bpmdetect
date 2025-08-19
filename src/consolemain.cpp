// SPDX-License-Identifier: GPL-3.0-or-later
#include "consolemain.h"
#include "track/trackffmpeg.h"

void consoleMain(QCommandLineParser &parser, const QStringList &files) {
    auto remove = parser.isSet(QStringLiteral("remove"));
    auto consoleProgress = !parser.isSet(QStringLiteral("no-progress"));
    auto detect = parser.isSet(QStringLiteral("detect"));
    auto format = parser.value(QStringLiteral("format"));
    auto save = parser.isSet(QStringLiteral("save"));
    auto detector = new SoundTouchBpmDetector();
    if (files.isEmpty()) {
        parser.showHelp(1);
    }
    for (const auto &file : files) {
        TrackFfmpeg track(file, false);
        if (!remove) {
            track.setConsoleProgress(consoleProgress);
            track.setRedetect(detect);
            track.setFormat(format);
            track.setDetector(detector);
            track.detectBpm();
            track.printBpm();
            if (save) {
                track.saveBpm();
            }
        } else {
            track.clearBpm();
        }
    }
    delete detector;
}
