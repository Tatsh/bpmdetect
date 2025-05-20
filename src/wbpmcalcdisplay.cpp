/***************************************************************************
     Copyright          : (C) 2009 by Martin Sakmar
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

#include "wbpmcalcdisplay.h"

#include <QPainter>

#include <QDebug>

WBPMCalcDisplay::WBPMCalcDisplay(QWidget *parent) : WPlot(parent) {
    m_winStart = 0;
}

WBPMCalcDisplay::~WBPMCalcDisplay() {
}

void WBPMCalcDisplay::setPeaks(std::vector<Peak> peaks) {
    m_peaks = peaks;
}

void WBPMCalcDisplay::updateData(const BPMCalculator *pCalc) {
    if (!pCalc)
        return;

    int winLen = 0;
    m_winStart = 0;
    const float *xcorr = pCalc->xcorrData(m_winStart, winLen);
    // find xcorr max value
    float max = 0.1, min = xcorr[m_winStart];
    for (int i = m_winStart; i < winLen; ++i) {
        if (xcorr[i] > max)
            max = xcorr[i];
        if (xcorr[i] < min)
            min = xcorr[i];
    }

    setMinValue(min);
    setMaxValue(max);
    setData(xcorr + m_winStart, winLen - m_winStart);
    m_peaks = pCalc->peaks();
}

void WBPMCalcDisplay::paintEvent(QPaintEvent *e) {
    WPlot::paintEvent(e);
    QPainter p(this);
    p.setWorldTransform(m_transform);
    //p.setWorldMatrix(m_matrix);
    QPen peakPen, bpmPen, corrbpmPen;
    peakPen.setColor(Qt::red);
    bpmPen.setColor(Qt::blue);
    corrbpmPen.setColor(Qt::yellow);
    p.setPen(peakPen);

    for (std::size_t i = 0; i < m_peaks.size(); ++i) {
        QPointF p1, p2, p3;
        Peak peak = m_peaks[i];

        p1 = m_points[peak.firstPos - m_winStart];
        p2 = m_points[peak.peakPos - m_winStart];
        p3 = m_points[peak.lastPos - m_winStart];

        p.setPen(peakPen);
        p.drawLine(p1, p2);
        p.drawLine(p2, p3);

        p.setPen(bpmPen);
        p.drawLine(QPointF(peak.massCenter - m_winStart, m_maxVal),
                   QPointF(peak.massCenter - m_winStart, m_minVal));

        p.setPen(corrbpmPen);
        p.drawLine(QPointF(peak.corrPos - m_winStart, m_maxVal),
                   QPointF(peak.corrPos - m_winStart, m_minVal));
    }
}
