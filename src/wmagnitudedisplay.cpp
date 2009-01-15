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

#include "wmagnitudedisplay.h"

#include <QPainter>

WMagnitudeDisplay::WMagnitudeDisplay(QWidget *parent) : QWidget(parent) {}

WMagnitudeDisplay::~WMagnitudeDisplay() {}

void WMagnitudeDisplay::setData(float *data, unsigned long size) {
    m_points.clear();
    for(unsigned long i = 0; i < size; ++i) {
        QPoint pt(i, data[i]);
        m_points.append(pt);
    }
}

void WMagnitudeDisplay::paintEvent(QPaintEvent* e) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    float scalex = (float) width() / 100.0f;
    if(m_points.size()) scalex = (float) width() / (float) m_points.size();
    float scaley = - ((float) height() - 0.1*height()) / 50.; //< FIXME: maximum value (instead of 50)

    p.translate(0, height() - 0.1 * height());
    p.scale(scalex, scaley);

    p.setPen(Qt::green);
    for(int i = 1; i < m_points.size(); ++i) {
        p.drawLine(m_points.at(i-1), m_points.at(i));
    }
}


