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
#include "dlgbpmdetect3.h"
#include <qapplication.h>
#include <qlocale.h>
#endif

#include <getopt.h>

#include "track.h"
#include "trackfmod.h"  // for FMOD system

#include <iostream>
#include <string>
using namespace std;

const char* version = "0.6.0";   ///< App version

void display_help() {
  printf("BPMDetect version %s\n\n", version);
  printf("Usage:\n bpmdetect [switches] [files]\n\n", version);
  printf("Switches:\n");
#ifndef NO_GUI
  printf("-c --console         - run in console mode\n");
#endif
  printf("-h --help            - show this help\n"
         "-s --save            - save BPMs to tags\n"
         "-d --detect          - redetect (do not print BPMs stored in tags)\n"
         "-r --remove          - remove stored BPMs from tags\n"
         "-p --noprogress      - disable progress display (console)\n"
         "-f --format <format> - set BPM format (default is 0.00)\n"
         "\n");
}

int main( int argc, char **argv ) {
  bool console  = false,
       redetect = false,
       bpmsave  = false,
       clear    = false,
       bformat  = false,
       progress = true;
  string format;

  static struct option long_options[] = {
  #ifndef NO_GUI
    {"console",    no_argument,       0, 'c'},
  #endif
    {"format",     required_argument, 0, 'f'},
    {"save",       no_argument,       0, 's'},
    {"detect",     no_argument,       0, 'd'},
    {"remove",     no_argument,       0, 'r'},
    {"noprogress", no_argument,       0, 'p'},
    {"help",       no_argument,       0, 'h'},
    {0, 0, 0, 0}
  };

  while ( true ) {
    int option_index = 0;
    int c;
    c = getopt_long(argc, argv, "csdrphf:",
                    long_options, &option_index);
    if(c < 0) break;
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
        // do not start GUI, just remove BPMs
        console = true;
        break;
      case 'p':
        progress = false;
        break;
      case 'f':
        bformat = true;
        format = optarg;
        break;
    #ifndef NO_GUI
      case 'c':
        console = true;
        break;
    #endif
      case '?': default:
        display_help();
        return 0;
    }
  }

#ifdef NO_GUI
  console = true;
  if(argc - optind < 1)
#else
  QStringList filelist;
  if(console && argc - optind < 1)
#endif
  { // no files passed
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
      Track* track = Track::createTrack(argv[idx]);
      if(!clear) {
        track->enableConsoleProgress(progress);
        track->setRedetect(redetect);
        if(bformat) track->setFormat(format);
        track->detectBPM();
        if(bpmsave) track->saveBPM();
        track->printBPM();
      } else {
        track->clearBPM();
      }
      delete track;
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
