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

#include "progressbar.h"
#include <iostream>

using namespace std;

/**
 * @brief Constructor
 * @param parent parent widget
 * @param name widget name
  */
ProgressBar::ProgressBar( QWidget *parent, const char *name )
    : QProgressBar( parent, name ) {
  setPercentageVisible( FALSE );
  setFrameShape( QFrame::Box );
  setFrameShadow( QFrame::Plain );
  setChange( FALSE );
  setEnabled( TRUE );
}

/// @brief Destructor
ProgressBar::~ProgressBar() {}

/**
 * @brief Receiving mouse press events
 * Set change status to true if progressbar is enabled
 * @param e is received mouse event
 */
void ProgressBar::mousePressEvent( QMouseEvent *e ) {
  if ( enabled() ) {
    if ( e->button() == Qt::LeftButton ) {
      setChange( TRUE );
      setProgress( ( e->pos().x() * totalSteps() ) / ( width() ) );
    } else if ( change() ) {
      setChange( FALSE );
    }
  }
}

/**
 * @brief Receiving mouse move events
 * Set current position if change status is true
 * and progressbar is enabled 
 * @param e is received mouse event
 */
void ProgressBar::mouseMoveEvent( QMouseEvent *e ) {
  if ( enabled() && change() ) {
    if ( e->pos().x() <= 0 )
      setProgress( 0 );
    else if ( e->pos().x() >= width() )
      setProgress( totalSteps() );
    else
      setProgress( ( e->pos().x() * totalSteps() ) / ( width() ) );
  }
}

/**
 * @brief Receiving mouse release events
 * Set Change status to false if progressbar is enabled
 * @param e is received mouse event
 */
void ProgressBar::mouseReleaseEvent( QMouseEvent *e ) {
  if ( enabled() ) {
    if ( e->button() == Qt::RightButton && e->state() == Qt::LeftButton )
      setChange( TRUE );
    else if ( change() && e->button() == Qt::LeftButton ) {
      setChange( FALSE );
      emit ( positionChanged( ( uint ) progress() ) );
    }
  }
}

/**
 * @brief Set total steps (length of track)
 * @param len is track length
 */
void ProgressBar::setLength( uint len ) {
  setTotalSteps( len );
}

/**
 * @brief Set current progressbar position
 * @param pos is position to set
 */
void ProgressBar::setPosition( uint pos ) {
  if ( ! change() )
    setProgress( pos );
}

/**
 * @brief Return change status
 * @return true if changing the position by mouse moving
 */
bool ProgressBar::change() {
  return chng;
}

/**
 * @brief Set change status
 * @param s change status
 */
void ProgressBar::setChange( bool s ) {
  chng = s;
}

/**
 * @brief Return eneble status
 * @return true if the progressbar is enabled
 */
bool ProgressBar::enabled() {
  return enable;
}

/**
 * @brief Set enabled status
 * @param s is true to enable, false to disable the progressbar
 */
void ProgressBar::setEnabled( bool s ) {
  enable = s;
}

/**
 * @brief Return length of track (total progressbar steps)
 * @return track length
 */
uint ProgressBar::length() {
  return totalSteps();
}

#include "progressbar.moc"
