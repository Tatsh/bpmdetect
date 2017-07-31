/***************************************************************************
                          wvumeter.h  -  description
                             -------------------
    begin                : Fri Jul 22 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WVUMETER_H
#define WVUMETER_H

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QPaintEvent>
#include <QTimer>

class WVuMeter : public QWidget  {
   Q_OBJECT
public: 
    WVuMeter(QWidget *parent=0);
    ~WVuMeter();
    void initPixmaps();
    /// value from 0 to 100
    void setValue(int value);

protected slots:
    void slotUpdatePeak();

private:
    void paintEvent(QPaintEvent *);
    void setPeak(int pos);

    int m_iValue;

    /** Current position */
    int m_iPos;
    int m_iNoPos;
    /** Associated pixmaps */
    QPixmap m_pixmapBack, m_pixmapVu;
    /** True if it's a horizontal vu meter */
    bool m_bHorizontal;

    int m_iPeakHoldSize;
    int m_iPeakFallStep;
    int m_iPeakHoldTime;
    int m_iPeakFallTime;
    int m_iPeakPos;

    QTimer m_qTimer;
};

#endif
