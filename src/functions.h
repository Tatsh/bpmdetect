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
#include <qfile.h>
#include <fmodex/fmod.h>
#include <fmodex/fmod_errors.h>
#include "BPMDetect.h"

#include <qsettings.h>

// Settings
extern QString set_format;
extern bool    set_skip;
extern bool    set_save;
extern bool    force, bpmsave;

using namespace std;
using namespace soundtouch;
extern FMOD_SYSTEM* SoundSystem;

struct TAGINFO {
  QString BPM;
  QString Artist;
  QString Title;
};

/// Initialize FMOD sound system
bool Init_FMOD_System();
/// Close FMOD sound system
void Close_FMOD_System();

/// Save BPM to file
void Save_BPM( QString file, float fBPM );
/// Correct detected BPM
float Correct_BPM( float BPM );
/// Print detected BPM to stdout
void Print_BPM( float BPM );
/// Detect BPM of one track
void Detect_BPM( QString filename );
/// Convert string to BPM (detect tag format)
float Str2BPM( QString str );
/// Convert BPM to string
QString BPM2str( float BPM );
/// Convert BPM to string using selected format
QString Format_BPM( float BPM );

/// Read ID3v2 tag (mp3 file)
TAGINFO GetTagInfoMPEG(QString file);
/// Read ID3v2 tag (wav file)
TAGINFO GetTagInfoWAV(QString file);

void Load_Settings();
void Save_Settings();

#endif  // _FUNCTIONS_H_
