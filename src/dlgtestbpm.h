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

#ifndef DLGTESTBPM_H
#define DLGTESTBPM_H

#include <QDialog>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QIODevice>
#include <QtCore/QLinkedList>
#include <QThread>

#if !defined(HAVE_FMOD)
#error FMOD Ex is required for testing BPMs (HAVE_FMOD not defined)
#endif

#include "trackfmod.h"

#include "ui_dlgtestbpmdlg.h"

class DlgTestBPM: public QDialog, public Ui_DlgTestBPMDlg {
    Q_OBJECT
public:
    DlgTestBPM(const QString file, const float bpm, QWidget *parent = 0 );
    ~DlgTestBPM();

protected slots:
    void setPos1();
    void setPos2();
    void setPos3();
    void setPos4();
    void setCustomPos( uint msec );
    void stop();
    /// Set number of beats to loop
    void setNumBeats( const QString& );

    void slotUpdateBpmList();

    void readBuffer();
    void decoderError(QAudioDecoder::Error error);
    void finishedDecoding();

private:
    float m_bpm;            ///< BPM to test
    QList<float> m_bpmList; ///< list of possible BPMs
    FMOD_SYSTEM  *system;   ///< FMOD sound system
    FMOD_SOUND   *sound;    ///< FMOD sound object
    FMOD_CHANNEL *channel;  ///< FMOD channel object

    qint64 lengthMS;

    QAudioDecoder *decoder;
    QAudioBuffer lastBuffer;
    QByteArray *buffer;
};

class AudioThread : public QThread {
    Q_OBJECT
    void run() {
        char *data = mBuf->data();
        qint64 dataRemaining = mBuf->size();
        struct timespec ts = { 0, (10 % 1000) * 1000 * 1000 };

        QAudioOutput output(mFormat);
        output.setBufferSize(512);
        QIODevice *dev = output.start();

        while (dataRemaining) {
            qint64 bytesWritten = dev->write(data, dataRemaining);
            dataRemaining -= bytesWritten;
            data += bytesWritten;
            nanosleep(&ts, NULL);
        }

        emit resultReady();
    }
public:
    AudioThread(QByteArray *buf, QAudioFormat format) {
        mBuf = buf;
        mFormat = format;
    }

signals:
    void resultReady();

private:
    QByteArray *mBuf;
    QAudioFormat mFormat;
};

#endif // DLGTESTBPM_H
