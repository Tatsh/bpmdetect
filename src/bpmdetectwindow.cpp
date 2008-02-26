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

#include <qmainwindow.h>
#include <qlocale.h>

#include "bpmdetectwindow.h"

static const char *icon_xpm[]={
"32 32 10 1",
". c None",
"b c #000000",
"a c #00ff00",
"h c #010101",
"g c #060606",
"# c #1955e7",
"f c #474747",
"c c #4a4a4a",
"d c #6c6c6c",
"e c #828282",
"................................",
"..............#######...........",
"............####....###.........",
"...........##..........#........",
"..........##...#####............",
".........#...###....#...........",
".........#..#...................",
"........#..#...####.............",
"..............#.................",
"aaaaa...........................",
"abbba...........................",
"abbba...........................",
"abbbaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"abbbbbbbaabbbbbbbaabbbbbbbbbbbba",
"acccccccaacccccccaacccccccccccca",
"adddddddaadddddddaadddddddddddda",
"aeeaaaeeaaeeaaaeeaaeeaaaeeaaaeea",
"affaaaffaaffaaaffaaffa.affa.affa",
"abbaaabbaaghaaabbaabba.abba.abba",
"abbbbbbbaabbbbbbbaabba.abba.abba",
"abbbbbbbaabbbbbbbaabba.abba.abba",
"aaaaaaaaaabbbaaaaaaaaa.aaaa.aaaa",
".........abbba..................",
".........abbba........#..#......",
".........aaaaa....####..#.......",
".......................##..#....",
"................#....###..#.....",
".................#####...##.....",
".............#..........##......",
"..............###....###........",
"................#######.........",
"................................"};

/// Constructor
BPMDetectWindow::BPMDetectWindow() : QMainWindow( 0, "BPMDetect" ) {
  bpmwid = new BPMDetectWidget( this );
  setCentralWidget( bpmwid );
  setCaption("BPM Detect");
  setIcon(icon_xpm);
}

/// Deatructor
BPMDetectWindow::~BPMDetectWindow() {
  delete bpmwid;
  bpmwid = 0;
}

/// Add files from list
void BPMDetectWindow::addFiles(QStringList &files) {
  bpmwid->addFiles( files );
}

#include "bpmdetectwindow.moc"
