// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtStateMachine/QState>
#include <QtStateMachine/QStateMachine>

#include "consolemain.h"
#include "track/trackffmpeg.h"

// This solution is adapted from https://stackoverflow.com/a/31389121/374110.
class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(const QStringList &files,
                    bool redetect,
                    const QString &format,
                    QObject *parent = nullptr)
        : QObject(parent), machine(this), s1(&machine), s2(&machine), s3(&machine), timer(this),
          track(QStringLiteral(""), false, this) {
        // track.setConsoleProgress(consoleProgress);
        track.setRedetect(redetect);
        track.setFormat(format);
        auto detector = new SoundTouchBpmDetector(this);
        connect(&track, &TrackFfmpeg::hasBpm, [this](bpmtype bpm) { track.printBpm(); });
        timer.setSingleShot(true);
        s1.addTransition(&track, &TrackFfmpeg::hasBpm, &s2);
        s1.addTransition(&timer, &QTimer::timeout, &s2);
        connect(&s1, &QState::entered, [this, files, detector] {
            for (const auto &file : files) {
                track.setFileName(file, false);
                track.setDetector(detector);
                track.detectBpm();
            }
            timer.start(10000); // 10 seconds per file
        });
        connect(&s2, &QState::entered, [this] {
            machine.stop();
            emit finished();
        });
        machine.setInitialState(&s1);
        machine.start();
    }

private:
    Q_SIGNAL void finished();

    QStateMachine machine;
    QState s1, s2, s3;
    QTimer timer;
    TrackFfmpeg track;
};

void waitForEventDispatcher(QThread *thread) {
    while (thread->isRunning() && !thread->eventDispatcher()) {
        QThread::yieldCurrentThread();
    }
}

template <typename Obj>
void instantiateInThread(QThread *thread,
                         const QStringList &files,
                         bool redetect,
                         const QString &format) {
    Q_ASSERT(thread);
    QObject *dispatcher = thread->eventDispatcher();
    Q_ASSERT(dispatcher); // The thread must have an event loop.
    QTimer::singleShot(0, dispatcher, [dispatcher, files, redetect, format]() {
        // This happens in the given thread.
        new Obj(files, redetect, format, dispatcher);
    });
}

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
            TrackFfmpeg track(file, false);
            track.clearBpm();
        }
        return 0;
    }
    struct _ : QThread {
        ~_() override {
            quit();
            wait();
        }
    } thread;
    thread.start();
    waitForEventDispatcher(&thread);
    instantiateInThread<Worker>(&thread, files, detect, format);
    return app.exec();
}

#include "consolemain.moc"
