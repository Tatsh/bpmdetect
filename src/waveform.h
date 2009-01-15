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

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "pasample.h"

class Waveform {
friend class WWaveform;
public:
    Waveform(unsigned long size);
    ~Waveform();

    void setSize(unsigned long size);
    unsigned long size() const;
    /// update should be called allways with the same number of samples (size)
    void update(const SAMPLE* buffer, unsigned long size, bool beat = false, int beatOffset = 0);

protected:
    /// Shift data to the left and add @a val value to the end
    void addValue(float val, bool beat, int beatOffset);
    /// Return pointer to array of values (use numValues to get size)
    const float* valueBuffer() const;
    const bool* beats() const;

private:
    unsigned long m_waveformBufSize;
    float* m_pWaveformBuffer;
    bool* m_pBeatBuffer;
};

#endif
