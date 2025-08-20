// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtStateMachine/QFinalState>
#include <QtStateMachine/QState>
#include <QtStateMachine/QStateMachine>

#include "consolemain.h"
#include "debug.h"
#include "track/track.h"

// This solution is adapted from https://stackoverflow.com/a/31389121/374110.
class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(const QStringList &files,
                    bool redetect,
                    const QString &format,
                    QObject *parent = nullptr)
        : QObject(parent), machine_(this), state1_(&machine_), state2_(&machine_), timer_(this),
          detector_(new SoundTouchBpmDetector(this)), printCount_(0),
          finalState_(new QFinalState(&machine_)) {
        timer_.setSingleShot(true);

        // QQueue<QState> stateQueue;
        // for (const auto &file : files) {
        //     stateQueue.enqueue(QState(&machine_));
        // }

        state1_.addTransition(this, &Worker::finished, &state2_);
        state2_.addTransition(&timer_, &QTimer::timeout, finalState_);
        connect(&state1_, &QState::entered, [this, redetect, format, files] {
            for (const auto &file : files) {
                Track track(file, false, this);
                track.setFormat(format);
                // track.setConsoleProgress(consoleProgress);
                track.setRedetect(redetect);
                track.setDetector(detector_);
                connect(&track, &Track::hasBpm, [this, &track, files](bpmtype bpm) {
                    qCDebug(gLogBpmDetect) << "BPM detected for" << track.fileName() << ":" << bpm;
                    track.printBpm();
                    // if (stateQueue.isEmpty()) {
                    //     emit finished();
                    // }
                });
                track.detectBpm();
            }
            qCDebug(gLogBpmDetect) << "Waiting for BPM detection to finish...";
            timer_.start(10000);
        });
        QObject::connect(&machine_,
                         &QStateMachine::finished,
                         QCoreApplication::instance(),
                         &QCoreApplication::quit);
        machine_.setInitialState(&state1_);
        machine_.start();
    }

private:
    Q_SIGNAL void finished();

    QStateMachine machine_;
    QState state1_, state2_, state3_;
    QTimer timer_;
    SoundTouchBpmDetector *detector_ = nullptr;
    QAtomicInt printCount_;
    QFinalState *finalState_;
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
    auto dispatcher = thread->eventDispatcher();
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
            Track track(file, false);
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
