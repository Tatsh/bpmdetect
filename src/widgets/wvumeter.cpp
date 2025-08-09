/***************************************************************************
                          wvumeter.cpp  -  description
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

#include "wvumeter.h"

#include <QPaintEvent>
#include <QPixmap>
#include <QtCore>
#include <QtDebug>
#include <QtGui>

#define DEFAULT_FALLTIME 10
#define DEFAULT_HOLDTIME 300

static const unsigned char VU_Off_data[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
    0x52, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x79, 0x08, 0x02, 0x00, 0x00, 0x00, 0x82,
    0xf8, 0x57, 0xc2, 0x00, 0x00, 0x00, 0x5c, 0x49, 0x44, 0x41, 0x54, 0x58, 0x85, 0xed, 0xd3,
    0x39, 0x0a, 0x80, 0x40, 0x10, 0x44, 0xd1, 0x1a, 0x10, 0xaf, 0xec, 0x92, 0x8c, 0x89, 0xcb,
    0xad, 0xcb, 0xc0, 0xbc, 0xbb, 0x13, 0x41, 0xf0, 0x57, 0xfc, 0x79, 0x59, 0xb5, 0xf5, 0x5a,
    0x14, 0x6e, 0x9f, 0x8e, 0x41, 0xd2, 0x38, 0x9f, 0x51, 0xf5, 0x44, 0x5b, 0x28, 0x75, 0x29,
    0x8f, 0x54, 0x89, 0x90, 0x90, 0xde, 0x90, 0x52, 0x2a, 0x7d, 0x8b, 0x6d, 0x24, 0xa4, 0xef,
    0x4a, 0xe9, 0x5d, 0x0a, 0x51, 0xaf, 0x44, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
    0x48, 0x48, 0xff, 0x91, 0x9a, 0xed, 0x24, 0x91, 0x6e, 0xc3, 0x5a, 0x89, 0xb2, 0x64, 0x4f,
    0x2d, 0xcb, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

static const unsigned char VU_On_data[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
    0x52, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x79, 0x08, 0x02, 0x00, 0x00, 0x00, 0x82,
    0xf8, 0x57, 0xc2, 0x00, 0x00, 0x00, 0x5b, 0x49, 0x44, 0x41, 0x54, 0x58, 0x85, 0xed, 0xd6,
    0x21, 0x0a, 0xc0, 0x40, 0x0c, 0x44, 0xd1, 0x1f, 0xd8, 0x3b, 0x97, 0x5d, 0xdf, 0xb2, 0xb7,
    0x4e, 0x45, 0x4d, 0x55, 0x22, 0x5b, 0xf1, 0x87, 0x81, 0xb8, 0xe7, 0x06, 0x12, 0x6b, 0x4f,
    0xca, 0x9c, 0xc7, 0xc5, 0xda, 0x33, 0xa1, 0x6a, 0xe6, 0x00, 0xa2, 0x94, 0x12, 0xc6, 0x73,
    0xea, 0x28, 0x29, 0x7d, 0x23, 0x75, 0xd4, 0x00, 0xa2, 0xa4, 0x32, 0x95, 0x94, 0x7e, 0x2c,
    0xb5, 0x73, 0x19, 0xd0, 0xcf, 0x45, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,
    0x49, 0xe9, 0x95, 0xc8, 0xf6, 0x65, 0x87, 0x1b, 0x8b, 0x86, 0x9f, 0xfe, 0xe7, 0xa5, 0x1b,
    0x7b, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

WVuMeter::WVuMeter(QWidget *parent) : QWidget(parent) {
    m_iPeakHoldSize = m_iPeakPos = 0;
    m_iValue = 0;
    initPixmaps();
    m_iPeakHoldSize = 4;
    m_iPeakFallStep = 1;
    m_iPeakHoldTime = DEFAULT_HOLDTIME;
    m_iPeakFallTime = DEFAULT_FALLTIME;
    connect(&m_qTimer, SIGNAL(timeout()), this, SLOT(slotUpdatePeak()));
    if (m_iPeakHoldSize > 0)
        m_qTimer.start(m_iPeakFallTime);
}

WVuMeter::~WVuMeter() {
}

void WVuMeter::initPixmaps() {
    QImage img;
    bool ok = img.loadFromData(VU_Off_data, sizeof(VU_Off_data), "png");
    if (!ok)
        qDebug() << "Can not load VU meter image (OFF)";
    m_pixmapBack = QPixmap::fromImage(img);
    if (m_pixmapBack.size() == QSize(0, 0))
        qDebug() << "WVuMeter: Error loading back pixmap";

    ok = img.loadFromData(VU_On_data, sizeof(VU_On_data), "png");
    if (!ok)
        qDebug() << "Can not load VU meter image (ON)";
    m_pixmapVu = QPixmap::fromImage(img);
    if (m_pixmapVu.size() == QSize(0, 0))
        qDebug() << "WVuMeter: Error loading vu pixmap";

    setFixedSize(m_pixmapBack.size());
    m_bHorizontal = false;
    if (m_bHorizontal)
        m_iNoPos = m_pixmapVu.width();
    else
        m_iNoPos = m_pixmapVu.height();

    update();
}

void WVuMeter::setValue(int value) {
    m_iValue = value;
    int idx = (int)(m_iValue * (float)(m_iNoPos) / 100.);
    // Range check
    if (idx > m_iNoPos)
        idx = m_iNoPos;
    else if (idx < 0)
        idx = 0;

    setPeak(idx);
    update();
}

void WVuMeter::setPeak(int pos) {
    if (pos > m_iPeakPos) {
        m_iPeakPos = pos - (pos % 2) + 1;
        m_qTimer.start(m_iPeakHoldTime);
    }
}

void WVuMeter::slotUpdatePeak() {
    if (m_iPeakPos > 0) {
        m_iPeakPos -= m_iPeakFallStep;
        update();
    }

    m_qTimer.setInterval(m_iPeakFallTime);
}

void WVuMeter::paintEvent(QPaintEvent *) {
    int idx = (int)(m_iValue * (float)(m_iNoPos) / 100.);

    // Range check
    if (idx > m_iNoPos)
        idx = m_iNoPos;
    else if (idx < 0)
        idx = 0;

    QPainter painter(this);
    // Draw back
    painter.drawPixmap(0, 0, m_pixmapBack);

    // Draw (part of) vu
    if (m_bHorizontal) {
        //This is a hack to fix something weird with horizontal VU meters:
        if (idx == 0)
            idx = 1;
        painter.drawPixmap(0, 0, m_pixmapVu, 0, 0, idx, m_pixmapVu.height());
        if (m_iPeakHoldSize > 0 && m_iPeakPos > 0) {
            painter.drawPixmap(m_iPeakPos - m_iPeakHoldSize,
                               0,
                               m_pixmapVu,
                               m_iPeakPos - m_iPeakHoldSize,
                               0,
                               m_iPeakHoldSize,
                               m_pixmapVu.height());
        }
    } else {
        painter.drawPixmap(
            0, m_iNoPos - idx, m_pixmapVu, 0, m_iNoPos - idx, m_pixmapVu.width(), idx);
        if (m_iPeakHoldSize > 0 && m_iPeakPos > 0) {
            painter.drawPixmap(0,
                               m_pixmapVu.height() - m_iPeakPos,
                               m_pixmapVu,
                               0,
                               m_pixmapVu.height() - m_iPeakPos,
                               m_pixmapVu.width(),
                               m_iPeakHoldSize);
        }
    }
}
