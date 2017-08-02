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
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QtCore/QLinkedList>
#include <QThread>

#include "ui_dlgtestbpmdlg.h"

class AudioThread;
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
    qint64 lengthUS;
    QAudioDecoder *decoder;
    QAudioBuffer lastBuffer;
    QByteArray *buffer;
    AudioThread *audioThread;
};

class AudioThread : public QThread {
    Q_OBJECT
    void run() {
        char *startptr, *data;
        startptr = data = mBuf->data();
        qint64 originalSize, dataRemaining;

        qint64 beatslen = (qint64)(((60000 * mBeatCount) / mBPM) * 1000);
        qint32 bytesForBeats = mFormat.bytesForDuration(beatslen);

        // FIXME Needs boundary checking
        dataRemaining = originalSize = bytesForBeats * mBeatCount;
        if (mPosUS) {
            qint32 skipBytes = mFormat.bytesForDuration(mPosUS);
            data += skipBytes;
            startptr += skipBytes;
        }

        while (dataRemaining) {
            if (output->state() != QAudio::ActiveState && output->state() != QAudio::IdleState) {
                break;
            }

            qint64 bytesWritten = dev->write(data, dataRemaining);
            dataRemaining -= bytesWritten;
            data += bytesWritten;

            if (dataRemaining <= 0) {
                data = startptr;
                dataRemaining = originalSize;
            }

            usleep(10);
        }
    }
public:
    AudioThread(QByteArray *buf, QAudioFormat format, float bpm, uint beatCount, qint64 posUS = 0) {
        mBuf = buf;
        mFormat = format;
        mBPM = bpm;
        mBeatCount = beatCount;
        mPosUS = posUS;

        output = new QAudioOutput(mFormat);
        output->setBufferSize(512);
        dev = output->start();
    }
    void stop() {
        output->stop();
    }

private:
    QByteArray *mBuf;
    QAudioFormat mFormat;
    float mBPM;
    uint mBeatCount;
    qint64 mPosUS;
    QAudioOutput *output;
    QIODevice *dev;
};

#endif // DLGTESTBPM_H
