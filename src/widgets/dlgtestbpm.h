/***************************************************************************
     Copyright          : (C) 2007 by Martin Sakmar
     e-mail             : martin.sakmar@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
