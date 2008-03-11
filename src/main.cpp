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
#include "functions.h"

using namespace std;

const char* description =
  "Simple BPM (beats per minute) detection utility";

const char* version = "0.5"; ///< App version

bool force   = false,          ///< false to skip tracks with stored BPM
     bpmsave = false;          ///< true to save BPM
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
         "-f     - redetect BPMs stored in tag\n");
}

int main( int argc, char **argv ) {
  opterr = 0;
  int c;
  bool console = false;

  while ((c = getopt (argc, argv, "csfh")) != -1) {
    switch (c) {
      case 's':
        bpmsave = 1;
        break;
      case 'f':
        force = 1;
        break;
      case 'h':
        display_help();
        return 0;
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
  #ifdef NO_GUI
    double BPM = Detect_BPM(argv[idx]);
    printBPM(BPM);
  #else
    if(console) {
      double BPM = Detect_BPM(argv[idx]);
      printBPM(BPM);
    } else filelist += argv[idx];
  #endif  // NO_GUI
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
    mainWin->addFiles( filelist );
    app.exec();
    delete mainWin; mainWin = 0;
  }
#endif  // NO_GUI

  Close_FMOD_System();
  return 0;
}
