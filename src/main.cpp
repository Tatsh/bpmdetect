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
 #ifdef USE_QT3
  #include "dlgbpmdetect3.h"
  #include <qapplication.h>
  #include <qlocale.h>
 #else
  #include "dlgbpmdetect.h"
  #include <QApplication>
 #endif
#endif

#include <getopt.h>

#include "trackproxy.h"
#include "trackfmod.h"  // for FMOD system

#include <iostream>
#include <string>
using namespace std;

const char* version = "0.6.1";   ///< App version

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
         "-n --min <value>     - minimum BPM (default 80)\n"
         "-x --max <value>     - maximum BPM (default 185)\n"
         "-l --limit           - do not return BPM above the range <min, max>\n"
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
    {"min",        required_argument, 0, 'n'},
    {"max",        required_argument, 0, 'x'},
    {"limit",      no_argument,       0, 'l'},
    {"help",       no_argument,       0, 'h'},
    {0, 0, 0, 0}
  };

  while ( true ) {
    int option_index = 0;
    int c, val;
    c = getopt_long(argc, argv, "csdrphf:n:x:l",
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
      case 'n':
        val = atoi(optarg);
        Track::setMinBPM(val);
        #ifdef DEBUG
          cerr << "Min BPM set to " << Track::getMinBPM() << endl;
        #endif
        break;
      case 'x':
        val = atoi(optarg);
        Track::setMaxBPM(val);
        #ifdef DEBUG
          cerr << "Max BPM set to " << Track::getMaxBPM() << endl;
        #endif
        break;
      case 'l':
        Track::setLimit(true);
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

  for(int idx = optind; idx < argc; idx++) {
    if(console) {
      if(optind != argc - 1)
        cout << "[" << idx + 1 - optind << "/" << argc - optind << "] " 
             << argv[idx] << endl;
      TrackProxy track(argv[idx]);
      if(!clear) {
        track.enableConsoleProgress(progress);
        track.setRedetect(redetect);
        if(bformat) track.setFormat(format);
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
    #if  defined(__WIN32) && defined(USE_QT3)
      app.setStyle("Keramik");
    #endif  // __WIN32
    #ifndef USE_QT3
      app.setStyle("plastique");
    #endif
    DlgBPMDetect *mainWin = new DlgBPMDetect();
  #ifdef USE_QT3
    app.setMainWidget( mainWin );
  #endif
    mainWin->show();
    mainWin->slotAddFiles( filelist );
    app.exec();
    delete mainWin; mainWin = 0;
  }
#endif  // NO_GUI

  TrackFMOD::closeFMODSystem();
  return 0;
}
