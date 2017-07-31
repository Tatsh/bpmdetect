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

#ifndef BEATINFO_H
#define BEATINFO_H

class BeatInfo {
public:
    BeatInfo();
    ~BeatInfo();

    unsigned long start() const;
    void setStart();
    void setStart(unsigned long msec);
    void setEnd();
    void setEnd(unsigned long msec);
    void setLength(unsigned long len = 1);
    unsigned long length() const;

    float energy() const;
    void setEnergy(float energy);
    void addEnergy(float energy);

    float getBPM(const BeatInfo* bi);

private:
    unsigned long m_start;  /// miliseconds 
    unsigned long m_length; /// beat length in miliseconds
    float m_energy;         /// beat energy
};

#endif
