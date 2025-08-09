/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
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

#pragma once

#define DEFAULT_THRESHOLD 1.0

class EnergyBeatDetector {
public:
    EnergyBeatDetector(unsigned long bufSize = 10);
    ~EnergyBeatDetector();

    void addValue(float val);
    void setBufferSize(unsigned long bufSize);
    bool isBeat();
    float beat();

    float getAverage() const;
    float getBeatMinimum() const;
    float getCurrentValue() const;
    float getPrevValue() const;
    float getThreshold() const;
    void setThreshold(float val);

protected:
    void updateValues();

private:
    float m_avgEnergy;
    float m_beatMin;
    float m_threshold;
    float m_currentValue;
    float m_prevValue;
    float envelopeAccu;

    unsigned long m_energyBufSize;
    unsigned long m_energyBufIdx; ///< index of new value in energy buffer
    float *m_pEnergyBuffer;       ///< sound energy buffer for last second
};
