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
#include <QDebug>

#include <QAudioDecoder>
#include <QAudioOutput>
#include <QTimer>

#include <iostream>

#include "dlgtestbpm.h"
#include "progressbar.h"

#include <fmodex/fmod_errors.h>

using namespace std;

DlgTestBPM::DlgTestBPM(const QString file, const float bpm, QWidget *parent) : QDialog( parent ) {
    setupUi(this);

    if (file.isEmpty()) {
        close();
    }

    channel = NULL;

    buffer = new QByteArray();

    decoder = new QAudioDecoder();
    decoder->setSourceFilename(file);
    decoder->start();

    connect(decoder, SIGNAL(bufferReady()), this, SLOT(readBuffer()));
    connect(decoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(decoderError(QAudioDecoder::Error)));
    connect(decoder, SIGNAL(finished()), this, SLOT(finishedDecoding()));



//     FMOD_RESULT result;
//     system = TrackFMOD::getFMODSystem();
//     channel = 0;
//     sound = 0;
//
    m_bpm = bpm;
    lblBPM->setText(QString::fromStdString(Track::bpm2str(bpm, "000.00")));
//
//     if (!system) close();
//
//     result = FMOD_System_CreateStream( system, file.toLocal8Bit(),
//                                        FMOD_SOFTWARE | FMOD_2D, 0, &sound );
//     if ( result != FMOD_OK ) {
//         cerr << "Error loading file " << file.toStdString() << " for testing BPM" << endl;
//         close();
//     }
//
//     FMOD_System_PlaySound( system, FMOD_CHANNEL_FREE,
//                            sound, TRUE, &channel );
//     uint length;
//     FMOD_Sound_GetLength( sound, &length, FMOD_TIMEUNIT_MS );
//     trackPosition->setLength( length );
//     FMOD_Channel_SetMode( channel, FMOD_LOOP_NORMAL );
//     connect( trackPosition, SIGNAL( positionChanged( uint ) ),
//              this, SLOT( setCustomPos( uint ) ) );

    slotUpdateBpmList();
}

void DlgTestBPM::readBuffer() {
    QAudioBuffer buf = lastBuffer = decoder->read();
    lengthMS += buf.duration();
    buffer->append(buf.data<const char>(), buf.byteCount());
}

void DlgTestBPM::finishedDecoding() {
    trackPosition->setLength(lengthMS);

    AudioThread *thread = new AudioThread(buffer, lastBuffer.format());
    thread->start();
}

void DlgTestBPM::decoderError(QAudioDecoder::Error error) {
    qDebug() << decoder->errorString();
    decoder->stop();

    // TODO Error dialog here
    close();
}

DlgTestBPM::~DlgTestBPM() {
    stop();
}

void DlgTestBPM::setPos1() {
    if ( channel != NULL ) {
        uint msec = trackPosition->length() / 5;
        unsigned long beatslen = ( unsigned long ) (
                                     ( 60000 * cbNBeats->currentText().toInt() ) / m_bpm );
        FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                                    msec + beatslen, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPaused( channel, FALSE );
        trackPosition->setPosition( msec );
    }
}

void DlgTestBPM::setPos2() {
    if ( channel != NULL ) {
        uint msec = ( trackPosition->length() * 2 ) / 5;
        unsigned long beatslen = ( unsigned long ) (
                                     ( 60000 * cbNBeats->currentText().toInt() ) / m_bpm );
        FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                                    msec + beatslen, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPaused( channel, FALSE );
        trackPosition->setPosition( msec );
    }
}

void DlgTestBPM::setPos3() {
    if ( channel != NULL ) {
        uint msec = ( trackPosition->length() * 3 ) / 5;
        unsigned long beatslen = ( unsigned long ) (
                                     ( 60000 * cbNBeats->currentText().toInt() ) / m_bpm );
        FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                                    msec + beatslen, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPaused( channel, FALSE );
        trackPosition->setPosition( msec );
    }
}

void DlgTestBPM::setPos4() {
    if ( channel != NULL ) {
        uint msec = ( trackPosition->length() * 4 ) / 5;
        unsigned long beatslen = ( unsigned long ) (
                                     ( 60000 * cbNBeats->currentText().toInt() ) / m_bpm );
        FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                                    msec + beatslen, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPaused( channel, FALSE );
        trackPosition->setPosition( msec );
    }
}

void DlgTestBPM::setCustomPos( uint msec ) {
    if ( channel != NULL ) {
        unsigned long beatslen = ( unsigned long ) (
                                     ( 60000 * cbNBeats->currentText().toInt() ) / m_bpm );
        FMOD_Channel_SetLoopPoints( channel, msec, FMOD_TIMEUNIT_MS,
                                    msec + beatslen, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPosition( channel, msec, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPaused( channel, FALSE );
    }
}

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
 * called when combobox currentItemChanged is emited
 * @param text is number of beats to loop (combobox currentText)
 */
void DlgTestBPM::setNumBeats( const QString &text ) {
    if ( channel != NULL ) {
        uint loopstart, loopend;
        FMOD_Channel_GetLoopPoints( channel, &loopstart, FMOD_TIMEUNIT_MS, &loopend, FMOD_TIMEUNIT_MS );
        unsigned long beatslen = ( unsigned long ) ( ( 60000 * text.toInt() ) / m_bpm );
        FMOD_Channel_SetLoopPoints( channel, loopstart, FMOD_TIMEUNIT_MS,
                                    loopstart + beatslen, FMOD_TIMEUNIT_MS );
        FMOD_Channel_SetPosition( channel, loopstart, FMOD_TIMEUNIT_MS );
    }
}

void DlgTestBPM::slotUpdateBpmList() {
    const int MIN_BPM = 45;
    const int MAX_BPM = 220;

    m_bpmList.clear();

    if(m_bpm > MIN_BPM && m_bpm < MAX_BPM) {
        float cbpm = m_bpm;
        while(cbpm / 2. > MIN_BPM) cbpm /= 2.;

        const float d = 0.25 * cbpm;
        while(cbpm-d > MIN_BPM) cbpm -= d;

        for(; cbpm < MAX_BPM; cbpm += d) {
            m_bpmList.append(cbpm);
        }
    }
    qDebug() << m_bpmList;
    qDebug() << "total list size:" << m_bpmList.size();
}

