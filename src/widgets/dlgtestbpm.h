// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QDialog>
#include <QIODevice>
#include <QList>
#include <QThread>

#include "dlgtestbpmplayer.h"
#include "ui_dlgtestbpmdlg.h"

class AudioThread;
/** Dialog to test the BPM. */
class DlgTestBPM : public QDialog, public Ui_DlgTestBPMDlg {
    Q_OBJECT
public:
    /** Constructor.
     * @param file File to test.
     * @param bpm BPM value.
     * @param parent Parent widget.
     */
    DlgTestBPM(const QString file, const float bpm, QWidget *parent = nullptr);
    ~DlgTestBPM() override;

protected Q_SLOTS:
    /** Set position 1. */
    void setPos1();
    /** Set position 2. */
    void setPos2();
    /** Set position 3. */
    void setPos3();
    /** Set position 4. */
    void setPos4();
    /** Set a custom position in milliseconds. */
    void setCustomPos(qint64 msec);
    /** Set number of beats to loop. @a s should contain an integer. */
    void setNumBeats(const QString &s);
    /** Update BPM list. */
    void slotUpdateBpmList();
    /** Set track position length. */
    void setTrackPositionLength(qint64);

private:
    DlgTestBPMPlayer *player;
    QList<float> m_bpmList;
    float m_bpm;
};
