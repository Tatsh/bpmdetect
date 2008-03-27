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

#ifndef QDROPLISTVIEW_H
#define QDROPLISTVIEW_H

#include <qlistview.h>


class QDropListView : public QListView {
  Q_OBJECT
public:
  QDropListView ( QWidget *parent = 0, const char *name = 0 );
  ~QDropListView();

public slots:
  /// Remove selected items from list
  void slotRemoveSelected();

protected:
  void keyPressEvent( QKeyEvent *e );
  void keyReleaseEvent( QKeyEvent *e );
  void dragEnterEvent( QDragEnterEvent *e );
  void dragMoveEvent( QDragMoveEvent *e );
  void dragLeaveEvent( QDragLeaveEvent *e );
  void dropEvent( QDropEvent *e );

signals:
  void keyPress( QKeyEvent *e );
  void keyRelease( QKeyEvent *e );
  void dragEnter( QDragEnterEvent *e );
  void dragMove( QDragMoveEvent *e );
  void dragLeave( QDragLeaveEvent *e );
  void drop( QDropEvent *e );
};

#endif
