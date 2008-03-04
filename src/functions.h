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

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <iostream>
#include <fmodex/fmod.h>
#include "BPMDetect.h"

// Settings
extern bool    force, bpmsave;

using namespace std;
using namespace soundtouch;
extern FMOD_SYSTEM* SoundSystem;

typedef struct taginfo_s {
  string BPM;
  string Artist;
  string Title;
} taginfo_t;

/// Initialize FMOD sound system
bool Init_FMOD_System();
/// Close FMOD sound system
void Close_FMOD_System();

/// Save BPM to file
void saveBPM( string filename, double dBPM, string format = "0.00" );
/// Correct detected BPM
double correctBPM( double dBPM, double min = 80., double max = 185. );
/// Print detected BPM to stdout
void printBPM( double dBPM, string format = "0.00" );
double Detect_BPM( string filename );
/// Convert string to BPM (detect tag format)
double str2bpm( string sBPM );
/// Convert BPM to string using selected format
string bpm2str( double dBPM, string format = "0.00");

/// Read ID3v2 tag (mp3 file)
taginfo_t getTagInfoMPEG(string filename);
/// Read ID3v2 tag (wav file)
taginfo_t getTagInfoWAV(string filename);


#endif  // _FUNCTIONS_H_
