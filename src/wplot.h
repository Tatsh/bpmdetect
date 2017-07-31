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

#ifndef WPLOT_H
#define WPLOT_H

#include <QWidget>
#include <QList>

class WPlot : public QWidget {
    Q_OBJECT
public:
    WPlot(QWidget *parent = 0);
    ~WPlot();

    void setData(const float* data, unsigned long size);
    void setMaxValue(float max);
    void setMinValue(float min);

protected:
    QTransform m_transform;
    float m_maxVal, m_minVal;
    QList<QPointF> m_points;

    void paintEvent(QPaintEvent* e);

};

#endif
