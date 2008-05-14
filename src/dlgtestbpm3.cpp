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

#include <qstring.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <iostream>

#include "dlgtestbpm3.h"
#include "progressbar3.h"

#include <fmodex/fmod_errors.h>

using namespace std;

/**
 * @brief Constroctor
 *
 * @param file track file name
 * @param dBPM detected BPM
 * @param parent parent widget
 * @param name widget name
 */
DlgTestBPM::DlgTestBPM( QString file, float dBPM,
                  QWidget *parent, const char *name )
    : DlgTestBPMDlg( parent, name ) {
  FMOD_RESULT result;
  system = TrackFMOD::getFMODSystem();
  bpm = dBPM;
  QString sbpm; sbpm.sprintf( "%.2f", bpm );
  sbpm.rightJustify( '0', 6 );
  lblBPM->setText( sbpm );
  channel = 0; sound = 0;
  if ( file.isEmpty() || !system)
    close();

  result = FMOD_System_CreateStream( system, file.local8Bit(),
                      FMOD_SOFTWARE | FMOD_2D, 0, &sound );
  if ( result != FMOD_OK ) {
    cerr << "Error loading file " << file << " for testing BPM" << endl;
    close();
  }

  FMOD_System_PlaySound( system, FMOD_CHANNEL_FREE,
                  sound, TRUE, &channel );
  uint length;
  FMOD_Sound_GetLength( sound, &length, FMOD_TIMEUNIT_MS );
  trackPosition->setLength( length );
  FMOD_Channel_SetMode( channel, FMOD_LOOP_NORMAL );
  connect( trackPosition, SIGNAL( positionChanged( uint ) ), 
           this, SLOT( setCustomPos( uint ) ) );
}

DlgTestBPM::~DlgTestBPM() {
  stop();
}

/// @brief Set 1st testing position
void DlgTestBPM::setPos1() {
  if ( channel != NULL ) {
    uint msec = trackPosition->length() / 5;
    unsigned long beatslen = ( unsigned long ) ( 
        ( 60000 * cbNBeats->currentText().toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                            msec + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPaused( channel, FALSE );
    trackPosition->setPosition( msec );
  }
}

/// @brief Set 2nd testing position
void DlgTestBPM::setPos2() {
  if ( channel != NULL ) {
    uint msec = ( trackPosition->length() * 2 ) / 5;
    unsigned long beatslen = ( unsigned long ) ( 
        ( 60000 * cbNBeats->currentText().toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                            msec + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPaused( channel, FALSE );
    trackPosition->setPosition( msec );
  }
}

/// @brief Set 3rd testing position
void DlgTestBPM::setPos3() {
  if ( channel != NULL ) {
    uint msec = ( trackPosition->length() * 3 ) / 5;
    unsigned long beatslen = ( unsigned long ) (
        ( 60000 * cbNBeats->currentText().toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                            msec + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPaused( channel, FALSE );
    trackPosition->setPosition( msec );
  }
}

/// @brief Set 4th testing position
void DlgTestBPM::setPos4() {
  if ( channel != NULL ) {
    uint msec = ( trackPosition->length() * 4 ) / 5;
    unsigned long beatslen = ( unsigned long ) ( 
        ( 60000 * cbNBeats->currentText().toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                            msec + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPaused( channel, FALSE );
    trackPosition->setPosition( msec );
  }
}

/// @brief Set custom testing position
void DlgTestBPM::setCustomPos( uint msec ) {
  if ( channel != NULL ) {
    unsigned long beatslen = ( unsigned long ) ( 
        ( 60000 * cbNBeats->currentText().toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                            msec + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPaused( channel, FALSE );
  }
}

/// @brief Stop audio track
void DlgTestBPM::stop() {
  if ( channel != NULL ) {
    FMOD_Channel_Stop(channel);
    channel = 0;
    FMOD_Sound_Release(sound);
    sound = 0;
  }
}

/**
 * @brief Set number of beats to loop
 * @param text is number of beats to loop given as QString
 */
void DlgTestBPM::setNumBeats( const QString &text ) {
  if ( channel != NULL ) {
    uint loopstart, loopend;
    FMOD_Channel_GetLoopPoints( channel, &loopstart, FMOD_TIMEUNIT_MS, &loopend, FMOD_TIMEUNIT_MS );
    unsigned long beatslen = ( unsigned long ) ( ( 60000 * text.toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, loopstart, FMOD_TIMEUNIT_MS,
                            loopstart + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, loopstart, FMOD_TIMEUNIT_MS );
  }
}

#include "dlgtestbpm3.moc"
