/***************************************************************************
     Copyright          : (C) 2009 by Martin Sakmar
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

#include <sys/time.h>

#include <QDebug>

#include "metronome.h"

Metronome::Metronome() {
    setInterval(1000);
    stop();
    setSync();
}

Metronome::~Metronome() {
}

void Metronome::setInterval(unsigned long msec) {
    if (msec < 100)
        msec = 100;
    m_interval = msec;
}

void Metronome::setBPM(float bpm) {
    m_interval = (unsigned long)(60000. / bpm);
}

void Metronome::setSync() {
    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec * 1000 + time.tv_usec / 1000;
    setSync(msec);
}

void Metronome::setSync(unsigned long msec) {
    m_syncTime = msec;
}

unsigned long Metronome::progress() const {
    if (!m_bStarted) {
        return 0;
    }

    struct timeval time;
    gettimeofday(&time, 0);
    unsigned long msec = time.tv_sec * 1000 + time.tv_usec / 1000;

    unsigned long syncms = m_syncTime % m_interval;
    unsigned long iprogress = (msec + m_interval - syncms) % m_interval;
    return iprogress;
}

float Metronome::progressPercent() const {
    return 100.0 * ((float)progress() / (float)m_interval);
}

void Metronome::start() {
    m_bStarted = true;
}

void Metronome::stop() {
    m_bStarted = false;
}
