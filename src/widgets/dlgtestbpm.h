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
class DlgTestBPM : public QDialog, public Ui_DlgTestBPMDlg {
    Q_OBJECT
public:
    DlgTestBPM(const QString file, const float bpm, QWidget *parent = 0);
    ~DlgTestBPM() override;

protected Q_SLOTS:
    void setPos1();
    void setPos2();
    void setPos3();
    void setPos4();
    void setCustomPos(uint msec);
    /// Set number of beats to loop
    void setNumBeats(const QString &s);
    void slotUpdateBpmList();
    void setTrackPositionLength(const qint64);

private:
    float m_bpm;            ///< BPM to test
    QList<float> m_bpmList; ///< list of possible BPMs
    DlgTestBPMPlayer *player;
};
