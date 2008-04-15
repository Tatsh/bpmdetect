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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <qwidget.h>
#include <qprogressbar.h>
#include <qevent.h>

/// @brief Progressbar controlled by mouse
class ProgressBar : public QProgressBar {
  Q_OBJECT

public:
  /// Constructor
  ProgressBar( QWidget *parent = 0, const char *name = 0 );
  /// Destructor
  ~ProgressBar();
  /// Return change status
  bool change();
  /// Return enabled status
  bool enabled();
  /// Return total steps (track length)
  uint length();

public slots:
  /// Set current position
  void setPosition( uint pos );
  /// Set total steps
  void setLength( uint len );
  /// Set enabled status
  void setEnabled( bool s );

protected slots:
  /// Receiving mouse press events
  void mousePressEvent( QMouseEvent *e );
  /// Receiving mouse move events
  void mouseMoveEvent( QMouseEvent *e );
  /// Receiving mouse release events
  void mouseReleaseEvent( QMouseEvent *e );
  /// Set change status
  void setChange( bool s );

signals:
  /// Current position changed
  void positionChanged( uint pos );

private:
  bool chng;    ///< true if left mouse button is down
  bool enable;  ///< enable status
};

#endif  //PROGRESSBAR_H
