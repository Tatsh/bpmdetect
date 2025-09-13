// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <QtCore/QEventLoop>
#include <QtMultimedia/QAudioDecoder>

#include "consolemain.h"
#include "debug.h"
#include "ffmpegutils.h"
#include "track/track.h"

#ifndef TESTING
#define SHOW_HELP(parser) parser.showHelp(1);
#else
#define SHOW_HELP(parser) return -1;
#endif

int consoleMain(QCoreApplication &app, QCommandLineParser &parser, const QStringList &files) {
    auto remove = parser.isSet(QStringLiteral("remove"));
    auto consoleProgress = !parser.isSet(QStringLiteral("no-progress"));
    auto detect = parser.isSet(QStringLiteral("detect"));
    auto format = parser.value(QStringLiteral("format"));
    auto save = parser.isSet(QStringLiteral("save"));
    if (files.isEmpty()) {
        SHOW_HELP(parser)
    }
    if (remove) {
        for (const auto &file : files) {
            Track(file).clearBpm();
        }
        return 0;
    }
    const auto detector = new SoundTouchBpmDetector();
    for (const auto &file : files) {
        QEventLoop loop;
        if (!isDecodableFile(file)) {
#ifndef TESTING
            qCWarning(gLogBpmDetect) << "File is not decodable, skipping:" << file;
#else
            std::cout << "File is not decodable, skipping: " << file.toStdString() << "\n";
#endif
            continue;
        }
        Track track(file, new QAudioDecoder(&app));
        if (track.hasValidBpm() && !detect) {
            track.printBpm();
            continue;
        }
        track.setFormat(format);
        detector->reset();
        track.setDetector(detector);
        QObject::connect(
            &track, &Track::hasBpm, [&track, &loop, &save, &consoleProgress](bpmtype bpm) {
                Q_UNUSED(bpm)
                if (consoleProgress) {
                    QTextStream(stdout) << "\r";
                }
                track.printBpm();
                if (save) {
                    track.saveBpm();
                }
                loop.quit();
                QTextStream(stdout).flush();
            });
        if (consoleProgress) {
            QObject::connect(&track, &Track::progress, [&track](quint64 pos, quint64 length) {
                const auto percent = length ? (pos * 100 / length) : 0;
                QTextStream(stdout) << "\r" << track.hostFileName() << ": " << percent << "%";
                if (pos >= length) {
                    // LCOV_EXCL_START
                    QTextStream(stdout) << "\n";
                    // LCOV_EXCL_STOP
                }
                QTextStream(stdout).flush();
            });
        }
        track.detectBpm();
        loop.exec();
    }
    delete detector;
    return 0;
}
