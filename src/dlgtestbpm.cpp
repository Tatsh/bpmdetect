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

#include <QString>
#include <QComboBox>
#include <QLabel>

#include <iostream>

#include "dlgtestbpm.h"
#include "progressbar.h"

#include <fmodex/fmod_errors.h>

using namespace std;

dlgTestBPM::dlgTestBPM( QString file, float dBPM, QWidget *parent )
    : QDialog( parent ) {
  setupUi(this);
  FMOD_RESULT result;
  system = TrackFMOD::getFMODSystem();
  bpm = dBPM;
  lblBPM->setText( QString::fromStdString(Track::bpm2str(dBPM, "000.00")));
  channel = 0; sound = 0;
  if ( file.isEmpty() || !system) close();

  result = FMOD_System_CreateStream( system, file.toLocal8Bit(),
                      FMOD_SOFTWARE | FMOD_2D, 0, &sound );
  if ( result != FMOD_OK ) {
    cerr << "Error loading file " << file.toStdString() << " for testing BPM" << endl;
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

dlgTestBPM::~dlgTestBPM() {
  stop();
}

void dlgTestBPM::setPos1() {
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

void dlgTestBPM::setPos2() {
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

void dlgTestBPM::setPos3() {
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

void dlgTestBPM::setPos4() {
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

void dlgTestBPM::setCustomPos( uint msec ) {
  if ( channel != NULL ) {
    unsigned long beatslen = ( unsigned long ) ( 
        ( 60000 * cbNBeats->currentText().toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                            msec + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPaused( channel, FALSE );
  }
}

void dlgTestBPM::stop() {
  if ( channel != NULL ) {
    FMOD_Channel_Stop(channel);
    channel = 0;
    FMOD_Sound_Release(sound);
    sound = 0;
  }
}

/**
 * @brief Set number of beats to loop
 * called when combobox currentItemChanged is emited
 * @param text is number of beats to loop (combobox currentText)
 */
void dlgTestBPM::setNumBeats( const QString &text ) {
  if ( channel != NULL ) {
    uint loopstart, loopend;
    FMOD_Channel_GetLoopPoints( channel, &loopstart, FMOD_TIMEUNIT_MS, &loopend, FMOD_TIMEUNIT_MS );
    unsigned long beatslen = ( unsigned long ) ( ( 60000 * text.toInt() ) / bpm );
    FMOD_Channel_SetLoopPoints( channel, loopstart, FMOD_TIMEUNIT_MS,
                            loopstart + beatslen, FMOD_TIMEUNIT_MS );
    FMOD_Channel_SetPosition( channel, loopstart, FMOD_TIMEUNIT_MS );
  }
}
