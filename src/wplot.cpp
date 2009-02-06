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

#include "wplot.h"

#include <QPainter>

WPlot::WPlot(QWidget *parent) : QWidget(parent) {
    setMinValue(0);
    setMaxValue(100);
}

WPlot::~WPlot() {}

void WPlot::setMaxValue(float maxVal) {
    m_maxVal = maxVal;
    if(m_maxVal < m_minVal) m_minVal = m_maxVal - 1;
}

void WPlot::setMinValue(float minVal) {
    m_minVal = minVal;
    if(m_maxVal < m_minVal) m_maxVal = m_minVal + 1;
}

void WPlot::setData(const float *data, unsigned long size) {
    m_points.clear();
    for(unsigned long i = 0; i < size; ++i) {
        QPointF pt(i, data[i]);
        m_points.append(pt);
    }
}

void WPlot::paintEvent(QPaintEvent* e) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    float valDiff = m_maxVal - m_minVal;
    if(valDiff < 1) valDiff = m_maxVal;
    float scalex = (float) width() / 100.0f;
    if(m_points.size() > 1) scalex = (float) width() / (float) (m_points.size()-1);
    float scaley = - ((float) height() - 0.02*height()) / valDiff;

    p.translate(0, height() - 0.02 * height());
    p.scale(scalex, scaley);

    p.setPen(Qt::green);
    for(int i = 1; i < m_points.size(); ++i) {
        QPointF p1 = m_points.at(i-1);
        QPointF p2 = m_points.at(i);
        if(m_minVal > 0) {
            p1.setY(p1.y() - m_minVal);
            p2.setY(p2.y() - m_minVal);
        }
        p.drawLine(p1, p2);
    }

    m_transform = p.worldTransform();
}


