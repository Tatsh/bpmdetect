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

#include <QMouseEvent>

using namespace std;

ProgressBar::ProgressBar( QWidget *parent )
    : QProgressBar( parent ) {
  setTextVisible( false );
  setChange( false );
  setEnabled( true );
}

ProgressBar::~ProgressBar() {}

void ProgressBar::mousePressEvent( QMouseEvent *e ) {
  if ( enabled() ) {
    if ( e->button() == Qt::LeftButton ) {
      setChange( true );
      setValue( ( e->pos().x() * maximum() ) / ( width() ) );
    } else if ( change() ) {
      setChange( false );
    }
  }
}

void ProgressBar::mouseMoveEvent( QMouseEvent *e ) {
  if ( enabled() && change() ) {
    if ( e->pos().x() <= 0 )
      setValue( 0 );
    else if ( e->pos().x() >= width() )
      setValue( maximum() );
    else
      setValue( ( e->pos().x() * maximum() ) / ( width() ) );
  }
}

void ProgressBar::mouseReleaseEvent( QMouseEvent *e ) {
  if ( enabled() ) {
    if ( e->buttons() & (Qt::RightButton | Qt::LeftButton) )
      setChange( true );
    else if ( change() && e->button() == Qt::LeftButton ) {
      setChange( false );
      emit ( positionChanged( ( uint ) value() ) );
    }
  }
}

void ProgressBar::setLength( uint len ) {
  setMaximum( len );
}

void ProgressBar::setPosition( uint pos ) {
  if ( ! change() )
    setValue( pos );
}

bool ProgressBar::change() {
  return chng;
}

void ProgressBar::setChange( bool s ) {
  chng = s;
}

bool ProgressBar::enabled() {
  return enable;
}

void ProgressBar::setEnabled( bool s ) {
  enable = s;
}

uint ProgressBar::length() {
  return maximum();
}
