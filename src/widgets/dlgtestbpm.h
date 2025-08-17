// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QtCore/QList>
#include <QtWidgets/QDialog>

#include "dlgtestbpmplayer.h"
#include "ui_dlgtestbpmdlg.h"

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
    DlgTestBpm(const QString file, const float bpm, QWidget *parent = nullptr);
    ~DlgTestBpm() override;

protected Q_SLOTS:
    /** Set a custom position in milliseconds. */
    void setCustomPos(int msec);
    /** Set number of beats to loop. @a s should contain an integer. */
    void setNumBeats(const QString &s);
    /** Update BPM list. */
    void slotUpdateBpmList();
    /** Set track position length. */
    void setTrackPositionLength(qint64);

private:
    void setPosFromButton(int msec);
    DlgTestBpmPlayer *player;
    QList<float> m_bpmList;
    float m_bpm;
};
