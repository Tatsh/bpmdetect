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

#include "wwaveform.h"
#include "waveform.h"

#include <QPainter>
#include <QDebug>

WWaveform::WWaveform(QWidget *parent) : QWidget(parent) {
    setWaveform(0);
    setAutoScale(false);
}

WWaveform::~WWaveform() {
}

void WWaveform::setWaveform(Waveform* pWaveform) {
    m_pWaveform = pWaveform;
}

void WWaveform::setAutoScale(bool autoScale) {
    m_bAutoScale = autoScale;
}

void WWaveform::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    if(!m_pWaveform) return;

    unsigned int size = m_pWaveform->size();
    const float* values = m_pWaveform->valueBuffer();
    const bool* beats = m_pWaveform->beats();
    float fwidth = ((float) width()) / (float) size;
    int maxHeight = height() / 2;
    float valscale = 1;
    if(m_bAutoScale) valscale = 1.0/m_pWaveform->getMaxValue();
    if(valscale > 10) valscale = 10;

    for(int i = 0; i < size; ++i) {
        QRect rect;
        rect.setX(fwidth * i);
        rect.setY(maxHeight - (values[i] * valscale * maxHeight));
        rect.setWidth((int) fwidth+1.0);
        rect.setHeight((int) (values[i] * valscale * maxHeight * 2));

        p.fillRect(rect, Qt::green);
        
        // paint detected beat
        if(beats[i]) {
            QRect r(fwidth * i, 0, 2, height());
            p.fillRect(r, Qt::red);
        }
    }

    // TODO: paint beats
    
}
