// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <QMouseEvent>

#include "progressbar.h"

using namespace std;

ProgressBar::ProgressBar(QWidget *parent) : QProgressBar(parent) {
    setTextVisible(false);
    setChange(false);
    setEnabled(true);
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::mousePressEvent(QMouseEvent *e) {
    if (enabled()) {
        if (e->button() == Qt::LeftButton) {
            setChange(true);
            setValue((e->pos().x() * maximum()) / (width()));
        } else if (change()) {
            setChange(false);
        }
    }
}

void ProgressBar::mouseMoveEvent(QMouseEvent *e) {
    if (enabled() && change()) {
        if (e->pos().x() <= 0)
            setValue(0);
        else if (e->pos().x() >= width())
            setValue(maximum());
        else
            setValue((e->pos().x() * maximum()) / (width()));
    }
}

void ProgressBar::mouseReleaseEvent(QMouseEvent *e) {
    if (enabled()) {
        if (e->buttons() & (Qt::RightButton | Qt::LeftButton))
            setChange(true);
        else if (change() && e->button() == Qt::LeftButton) {
            setChange(false);
            emit(positionChanged(static_cast<qint64>(value())));
        }
    }
}

void ProgressBar::setLength(uint len) {
    setMaximum(static_cast<int>(len));
}

void ProgressBar::setPosition(uint pos) {
    if (!change())
        setValue(static_cast<int>(pos));
}

bool ProgressBar::change() {
    return chng;
}

void ProgressBar::setChange(bool s) {
    chng = s;
}

bool ProgressBar::enabled() {
    return enable;
}

void ProgressBar::setEnabled(bool s) {
    enable = s;
}

uint ProgressBar::length() {
    return static_cast<uint>(maximum());
}
