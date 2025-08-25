// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QBuffer>
#include <QtMultimedia/QAudioBuffer>
#include <QtWidgets/QDialog>

#include "ui_dlgtestbpmdlg.h"
#include "utils.h"

class QAudioDecoder;
class QAudioFormat;
class QMediaPlayer;

class LoopingBuffer : public QBuffer {
    Q_OBJECT
public:
    explicit LoopingBuffer(QObject *parent = nullptr);
    qint64 readData(char *data, qint64 maxlen) override;
    bool atEnd() const override {
        return false;
    }
};

/** Dialog to test the BPM. */
class DlgTestBpm : public QDialog, public Ui_DlgTestBpmDlg {
    Q_OBJECT
#ifdef TESTING
    friend class DlgTestBpmTest;
#endif
public:
    /** Constructor.
     * @param file File to test.
     * @param bpm BPM value.
     * @param parent Parent widget.
     */
    DlgTestBpm(QString file, bpmtype bpm, QWidget *parent = nullptr);
    ~DlgTestBpm() override;

protected Q_SLOTS:
    /** Set a custom position in milliseconds. */
    void setCustomPos(int msec);
    /** Set number of beats to loop. @a s should contain an integer. */
    void setNumBeats(const QString &s);
    /** Set track position length. */
    void setTrackPositionLength(qint64);

private:
    void setPosFromButton(int msec);
    LoopingBuffer *loopingBuffer_ = nullptr;
    QAudioBuffer lastBuffer_;
    QAudioDecoder *decoder_ = nullptr;
    QAudioFormat format_;
    QByteArray completeBuffer_;
    QMediaPlayer *player_ = nullptr;
    bpmtype bpm_ = 0;
    qint64 lengthUs_ = 0;
};
