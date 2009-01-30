/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
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

#ifndef DLGREALTIME_H
#define DLGREALTIME_H

#include <QDialog>
#include "ui_dlgrealtimedlg.h"

class AudioAnalyzer;
class AudioInput;

class DlgRealtime : public QDialog, public Ui_DlgRealtimeDlg {
    Q_OBJECT
public:
    DlgRealtime(QWidget *parent = 0);
    ~DlgRealtime();

protected slots:
    void slotUpdateVuMeters(int val);
    void slotUpdate();
    void slotShowBeat(bool beat);

private:
    AudioInput* m_pInput;
    AudioAnalyzer* m_pAnalyzer;

};

#endif
