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

#include "wenergybeatdisplay.h"
#include "core/energybeatdetector.h"

#include <QPainter>

WEnergyBeatDisplay::WEnergyBeatDisplay(QWidget *parent) : QWidget(parent) {
}

WEnergyBeatDisplay::~WEnergyBeatDisplay() {
}

void WEnergyBeatDisplay::addBeatDetector(EnergyBeatDetector *pDetector) {
    if (!pDetector)
        return;

    m_qBeatDetList.append(pDetector);
}

void WEnergyBeatDisplay::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    if (m_qBeatDetList.size() == 0)
        return;
    int w = width() / m_qBeatDetList.size();
    if (w < 1)
        w = 1;

    for (int i = 0; i < m_qBeatDetList.size(); ++i) {
        EnergyBeatDetector *pDetector = m_qBeatDetList.at(i);
        float avg = pDetector->getAverage();
        float cur = pDetector->getCurrentValue();
        float threshold = pDetector->getThreshold();
        float beatmin = pDetector->getBeatMinimum();
        float beat = pDetector->beat() * 8.;
        bool bbeat = pDetector->isBeat();
        int x = i * w;
        int cw = w;
        if (w > 3) {
            x += 1;
            cw -= 2;
        }

        if (beat < 0)
            beat = 0;
        if (beat > 255)
            beat = 255;

#define MAXVAL 75

        // current
        int h = cur * height() / MAXVAL;
        QRect rcur(x, height() - h, cw, h);
        p.fillRect(rcur, Qt::blue);

        // average
        h = avg * height() / MAXVAL;
        QRect ravg(x, height() - h + 2, cw, 4);
        p.fillRect(ravg, Qt::red);

        // beatmin
        h = beatmin * height() / MAXVAL;
        QRect rmin(x, height() - h + 2, cw, 4);
        p.fillRect(rmin, Qt::green);

        // threshold
        h = threshold * height() / MAXVAL;
        QRect rthr(x, height() - h + 2, cw, 4);
        p.fillRect(rthr, Qt::darkBlue);

        // beat
        if (bbeat) {
            QColor color(255, 255 - beat, 0);
            QRect rbeat(x, 0, cw, 5);
            p.fillRect(rbeat, color);
        }
    }
}
