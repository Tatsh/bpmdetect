// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QList>
#include <QtWidgets/QDialog>

#include "dlgtestbpmplayer.h"
#include "ui_dlgtestbpmdlg.h"
#include "utils.h"

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
    DlgTestBpm(QString file, bpmtype bpm, DlgTestBpmPlayer *player, QWidget *parent = nullptr);
    ~DlgTestBpm() override;

Q_SIGNALS:
    /** Signal emitted when the dialog is closed with a new BPM value. */
    void newBpmOnClose(bpmtype bpm);

public Q_SLOTS:
    void accept() override;

protected Q_SLOTS:
    /** Set a custom position in milliseconds. */
    void setCustomPos(int msec);
    /** Set number of beats to loop. @a s should contain an integer. */
    void setNumBeats(const QString &s);
    /** Set track position length. */
    void setTrackPositionLength(qint64);

private:
    void setPosFromButton(int msec);
    DlgTestBpmPlayer *player_;
    bool modifiedBpm_ = false;
    bpmtype bpm_;
};
