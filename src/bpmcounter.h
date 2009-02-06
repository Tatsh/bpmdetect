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

#ifndef BPMCOUNTER_H
#define BPMCOUNTER_H

#include <list>

#define BUFFER_SIZE 15

class BeatInfo;

class BPMCounter {
public:
    BPMCounter();
    ~BPMCounter();
    static double correctBPM(double dBPM, float min = 90., float max = 200., bool blimit = false);
    
    void reset();
    void addBeat();
    float getBPM() const;
    float getError() const;
    unsigned long getBeatCount() const;
    
    void setMinBPM(unsigned int minBPM = 80);
    void setMaxBPM(unsigned int maxBPM = 180);

protected:
    void calcBPM();

private:
    unsigned long m_tstart, m_tcurr, m_tprev, m_ntstart;
    unsigned long m_beatCount, m_nbeatCount;
    
    float m_fBPM, m_fError;
    
    unsigned int m_minBPM, m_maxBPM;
    float m_bpmbuffer[BUFFER_SIZE];
};

#endif
