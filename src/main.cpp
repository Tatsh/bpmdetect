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

#include "trackproxy.h"
#include "trackfmod.h"  // for FMOD system

#include <iostream>
using namespace std;

const char* version = "0.6";   ///< App version

void display_help() {
  printf("BPMDetect %s\n", version);
  printf("Usage:\n bpmdetect [switches] [files]\n\n", version);
  printf("Switches:\n");
#ifndef NO_GUI
  printf("-c     - console mode\n");
#endif
  printf("-h     - show this help\n"
         "-s     - save BPMs to tags\n"
         "-d     - detect (do not print BPMs stored in tags)\n"
         "-r     - remove stored BPMs from tags\n"
         "-p     - disable progress display (console)\n"
         "\n");
}

int main( int argc, char **argv ) {
  opterr = 0;
  int c;
  bool console  = false,
       redetect = false,
       bpmsave  = false,
       clear    = false,
       progress = true;

  while ((c = getopt (argc, argv, "csdrph")) != -1) {
    switch (c) {
      case 's':
        bpmsave = true;
        break;
      case 'd':
        redetect = true;
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

  if ( !TrackFMOD::initFMODSystem() ) {
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

  TrackFMOD::closeFMODSystem();
  return 0;
}
