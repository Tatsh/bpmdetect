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

#include "qdroplistview3.h"

QDropListView::QDropListView ( QWidget *parent, const char *name )
    : QListView ( parent, name ) {
  setAcceptDrops(true);
  setShowSortIndicator(true);
  setAllColumnsShowFocus(true);
  setResizeMode(QListView::NoColumn);
  setSelectionMode(QListView::Extended);
}

QDropListView::~QDropListView() {}

void QDropListView::slotRemoveSelected() {
  QListViewItemIterator it( this );
  for ( ; it.current(); it++ ) {
    if ( it.current() != firstChild() && it.current() ->isSelected() ) {
      removeItem( it.current() ); it--;
    }
  }
  if ( firstChild() && firstChild()->isSelected() )
    removeItem( firstChild() );
  clearSelection();
}

void QDropListView::keyPressEvent( QKeyEvent *e ) {
  if(e->key() == Qt::Key_Delete) slotRemoveSelected();
  else QListView::keyPressEvent(e);
  emit keyPress(e);
}

void QDropListView::keyReleaseEvent( QKeyEvent *e ) {
  QListView::keyReleaseEvent(e);
  emit keyRelease(e);
}

void QDropListView::dragEnterEvent( QDragEnterEvent *e ) {
  e->accept(true);
  emit dragEnter(e);
}

void QDropListView::dragMoveEvent( QDragMoveEvent *e ) {
  e->accept(true);
  emit dragMove(e);
}

void QDropListView::dragLeaveEvent( QDragLeaveEvent *e ) {
  emit dragLeave(e);
}

void QDropListView::dropEvent( QDropEvent *e ) {
  e->accept(true);
  emit drop(e);
}

#include "qdroplistview3.moc"
