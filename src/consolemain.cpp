// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QEventLoop>
#include <QtMultimedia/QAudioDecoder>

#include "consolemain.h"
#include "debug.h"
#include "ffmpegutils.h"
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
    const auto detector = new SoundTouchBpmDetector(&app);
    for (const auto &file : files) {
        QEventLoop loop;
        if (!isDecodableFile(file)) {
            qCDebug(gLogBpmDetect) << "File is not decodable, skipping:" << file;
            continue;
        }
        Track track(file, new QAudioDecoder(&app));
        if (track.hasValidBpm() && !detect) {
            track.printBpm();
            track.deleteLater();
            continue;
        }
        track.setFormat(format);
        detector->reset();
        track.setDetector(detector);
        QObject::connect(
            &track, &Track::hasBpm, [&track, &loop, &save, &consoleProgress](bpmtype bpm) {
                if (consoleProgress) {
                    QTextStream(stdout) << "\r";
                }
                track.printBpm();
                if (save) {
                    track.saveBpm();
                }
                loop.quit();
                track.deleteLater();
                QTextStream(stdout).flush();
            });
        if (consoleProgress) {
            QObject::connect(&track, &Track::progress, [&track](quint64 pos, quint64 length) {
                const auto percent = length ? (pos * 100 / length) : 0;
                QTextStream(stdout) << "\r" << track.fileName() << ": " << percent << "%";
                if (pos >= length) {
                    QTextStream(stdout) << "\n";
                }
                QTextStream(stdout).flush();
            });
        }
        track.detectBpm();
        loop.exec();
    }
    return 0;
}
