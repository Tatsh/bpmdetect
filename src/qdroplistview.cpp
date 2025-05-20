/***************************************************************************
     Copyright          : (C) 2007 by Martin Sakmar
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

#include "qdroplistview.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>

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
