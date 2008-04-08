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

#ifndef NO_GUI
#include "dlgbpmdetect.h"
#include <qapplication.h>
#include <qlocale.h>
#endif

#include <unistd.h>

#include <fmodex/fmod.h>
#include <fmodex/fmod_errors.h>

#include "trackproxy.h"

#include <iostream>
using namespace std;

const char* version = "0.6";   ///< App version
FMOD_SYSTEM* SoundSystem = 0;  ///< FMOD sound system object


void display_help() {
  printf("BPMDetect %s\n", version);
  printf("Usage:\n bpmdetect [switches] [files]\n\n", version);
  printf("Switches:\n");
#ifndef NO_GUI
  printf("-c     - console mode\n");
#endif
  printf("-h     - show this help\n"\
         "-s     - save BPM to tag\n"\
         "-d     - detect (do not use BPMs stored in tag)\n"
         "-r     - remove stored BPMs\n",
         "-p     - disable proggress\n");
}


bool Init_FMOD_System() {
  FMOD_RESULT result;
  unsigned int version;
  int numdrivers = 0;
#ifdef DEBUG
  clog << "Initializing FMOD sound system" << endl;
#endif
  if(SoundSystem) return true;

  // create FMOD system
  result = FMOD_System_Create( &SoundSystem );
  if( result != FMOD_OK ) {
    cerr << FMOD_ErrorString(result) << endl;
    return false;
  }
  // check FMOD version
  result = FMOD_System_GetVersion(SoundSystem, &version);
#ifdef DEBUG
  if(result != FMOD_OK) {
    clog << "Can not get FMOD version" << endl;
  } else if (version < FMOD_VERSION) {
    fprintf(stderr, "Warning: You are using an old version of FMOD (%08x)."
                    "This program requires %08x\n", version, FMOD_VERSION);
  } else fprintf(stderr, "FMOD version: %08x\n", version);
#endif
  result = FMOD_System_GetNumDrivers(SoundSystem, &numdrivers);
#ifdef DEBUG
  if(result != FMOD_OK) clog << "Can't get number of drivers" << endl;
#endif
  if(numdrivers > 0) {
    for( int i = 0; i < numdrivers; ++i ) {
      result = FMOD_System_SetDriver(SoundSystem, i);
      if(result == FMOD_OK) {
        char name[256];
        #if (FMOD_VERSION >= 0x00041100)
          result = FMOD_System_GetDriverInfo(SoundSystem, i, name, 256, 0);
        #else
          result = FMOD_System_GetDriverName(SoundSystem, i, name, 256);
        #endif
        #ifdef DEBUG
        if(result == FMOD_OK) clog << "Driver: " << name << endl;
        #endif
        break;
      }
    }
  } else {
    cerr << "No soundcard found" << endl;
    cerr << FMOD_ErrorString(result) << endl;
    return false;
  }

  // init system
  result = FMOD_System_Init( SoundSystem, 1, FMOD_INIT_NORMAL, 0 );
  if ( result == FMOD_OK ) {
    return true;
  } else {
    cerr << "System init: " << FMOD_ErrorString(result) << endl;
    return false;
  }
}

/// @brief Close FMOD sound system
void Close_FMOD_System() {
  if(!SoundSystem) return;
#ifdef DEBUG
  clog << "Closing FMOD sound system" << endl;
#endif
  FMOD_CHANNELGROUP* masterchgrp = 0;
  if(FMOD_OK == FMOD_System_GetMasterChannelGroup(SoundSystem, &masterchgrp)) {
    FMOD_ChannelGroup_Stop(masterchgrp);
    FMOD_ChannelGroup_SetVolume(masterchgrp, 0.0);
    FMOD_ChannelGroup_Release(masterchgrp);
    masterchgrp = 0;
  }
  FMOD_System_Close(SoundSystem);
  FMOD_System_Release(SoundSystem);
  SoundSystem = 0;
}

int main( int argc, char **argv ) {
  opterr = 0;
  int c;
  bool console  = false,
       redetect = false,
       bpmsave  = false,
       clear    = false,
       progress = true;

  while ((c = getopt (argc, argv, "csdrh")) != -1) {
    switch (c) {
      case 's':
        bpmsave = 1;
        break;
      case 'd':
        redetect = 1;
        break;
      case 'h':
        display_help();
        return 0;
      case 'r':
        clear = true;
        // do not start GUI, just clear BPMs
        console = true;
        break;
      case 'p':
        progress = false;
        break;
    #ifndef NO_GUI
      case 'c':
        console = true;
        break;
    #endif
      case '?':
      #ifdef NO_GUI
        fprintf (stderr, "Unknown option '-%c'.\n", optopt);
        display_help();
        return 1;
      #endif
      default:
        break;
    }
  }

#ifdef NO_GUI
  console = true;
  if(argc - optind < 1)
#else
  QStringList filelist;
  if(console && argc - optind < 1)
#endif
  {
    display_help();
    return 0;
  }

  if ( !Init_FMOD_System() ) {
    cerr << "Error: Your soundcard is either busy or not present" << endl;
    return 1;
  }

  for(int idx = optind; idx < argc; idx++) {
    if(console) {
      if(optind != argc - 1)
        cout << "[" << idx + 1 - optind << "/" << argc - optind << "] "
             << argv[idx] << endl;
    }

    if(console) {
      TrackProxy track(argv[idx]);
      if(!clear) {
        track.enableConsoleProgress(progress);
        track.setRedetect(redetect);
        track.detectBPM();
        if(bpmsave) track.saveBPM();
        track.printBPM();
      } else {
        track.clearBPM();
      }
    } else {
    #ifndef NO_GUI 
      filelist += argv[idx];
    #endif  // NO_GUI
    }
  }

#ifndef NO_GUI
  if(!console) {
    QApplication app(argc, argv);
  
    dlgBPMDetect *mainWin = new dlgBPMDetect();
    app.setMainWidget( mainWin );
    #ifdef __WIN32
      app.setStyle("Keramik");
    #endif  // __WIN32
    mainWin->show();
    mainWin->slotAddFiles( filelist );
    app.exec();
    delete mainWin; mainWin = 0;
  }
#endif  // NO_GUI

  Close_FMOD_System();
  return 0;
}
