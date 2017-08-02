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
#include "track.h"

using namespace std;

DlgTestBPM::DlgTestBPM(const QString file, const float bpm, QWidget *parent) : QDialog( parent ) {
    setupUi(this);

    if (file.isEmpty()) {
        close();
    }

    m_bpm = bpm;
    buffer = new QByteArray();

    decoder = new QAudioDecoder();
    decoder->setSourceFilename(file);
    decoder->start();

    connect(decoder, SIGNAL(bufferReady()), this, SLOT(readBuffer()));
    connect(decoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(decoderError(QAudioDecoder::Error)));
    connect(decoder, SIGNAL(finished()), this, SLOT(finishedDecoding()));

    lblBPM->setText(QString::fromStdString(Track::bpm2str(bpm, "000.00")));

    slotUpdateBpmList();
}

void DlgTestBPM::readBuffer() {
    QAudioBuffer buf = lastBuffer = decoder->read();
    lengthUS += buf.duration();
    buffer->append(buf.data<const char>(), buf.byteCount());
}

void DlgTestBPM::finishedDecoding() {
    trackPosition->setLength(lengthUS);

    qDebug() << lengthUS;

    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt());
    audioThread->start();
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
    uint msec = trackPosition->length() / 5;
    stop();
    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt(), (qint64)(lengthUS * 0.2));
    audioThread->start();
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setPos2() {
    uint msec = ( trackPosition->length() * 2 ) / 5;
    stop();
    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt(), (qint64)(lengthUS * 0.4));
    audioThread->start();
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setPos3() {
    uint msec = ( trackPosition->length() * 3 ) / 5;
    stop();
    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt(), (qint64)(lengthUS * 0.6));
    audioThread->start();
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setPos4() {
    uint msec = ( trackPosition->length() * 4 ) / 5;
    stop();
    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt(), (qint64)(lengthUS * 0.8));
    audioThread->start();
    trackPosition->setPosition( msec );
}

void DlgTestBPM::setCustomPos( uint msec ) {
    stop();
    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt(), msec);
    audioThread->start();
}

void DlgTestBPM::stop() {
    audioThread->stop();
    audioThread->wait();
}

/**
 * @brief Set number of beats to loop
 * called when combobox currentItemChanged is emited
 * @param text is number of beats to loop (combobox currentText)
 */
void DlgTestBPM::setNumBeats( const QString &text ) {
    audioThread->stop();

    audioThread = new AudioThread(buffer, lastBuffer.format(), m_bpm, cbNBeats->currentText().toInt());
    audioThread->start();
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

