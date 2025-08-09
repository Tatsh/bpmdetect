// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>

#include "qdroplistview.h"

QDropListView::QDropListView(QWidget *parent) : QTreeWidget(parent) {
    setAcceptDrops(true);
    setAllColumnsShowFocus(true);
}

QDropListView::~QDropListView() {
}

void QDropListView::slotRemoveSelected() {
    QList<QTreeWidgetItem *> items = selectedItems();
    for (int i = 0; i < items.size(); ++i) {
        delete items.at(i);
    }
}

void QDropListView::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Delete)
        slotRemoveSelected();
    else
        QTreeWidget::keyPressEvent(e);
    emit keyPress(e);
}

void QDropListView::keyReleaseEvent(QKeyEvent *e) {
    QTreeWidget::keyReleaseEvent(e);
    emit keyRelease(e);
}

void QDropListView::dragEnterEvent(QDragEnterEvent *e) {
    e->accept();
    emit dragEnter(e);
}

void QDropListView::dragMoveEvent(QDragMoveEvent *e) {
    e->accept();
    emit dragMove(e);
}

void QDropListView::dragLeaveEvent(QDragLeaveEvent *e) {
    emit dragLeave(e);
}

void QDropListView::dropEvent(QDropEvent *e) {
    e->accept();
    emit drop(e);
}
