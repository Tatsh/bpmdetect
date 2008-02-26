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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qapplication.h>
#include <qprogressbar.h>
#include "qdroplistview.h"

#ifdef HAVE_KDE
#include <kaboutapplication.h>
#endif

#include "bpmdetectwidget.h"
#include "testbpm.h"

#include "functions.h"

static const char *topxpm[]={
"183 16 111 2",
"#C c #002242",
".B c #002546",
"#B c #002b51",
".z c #003054",
"#p c #00345a",
".y c #00375e",
"#o c #003a62",
"#n c #003d67",
".x c #00426c",
".w c #004470",
".v c #004874",
".u c #004d7a",
".t c #00507e",
".s c #005483",
".r c #005787",
".q c #005b8b",
".p c #005e90",
".o c #006294",
".d c #006699",
".A c #01284b",
"#q c #02679a",
"#A c #053256",
"#z c #073458",
"#R c #083c62",
".e c #086b9d",
"#m c #094b75",
"#y c #0a385c",
"#O c #0b537e",
".K c #0d314e",
"#x c #0d3b5e",
"#Q c #0d5f8b",
"#t c #0e3c60",
"#P c #0e4e77",
".f c #0e6e9f",
"#l c #115179",
".Y c #1170a0",
"#u c #124163",
"#S c #1271a1",
"#s c #14476b",
"#v c #154566",
"#w c #164768",
"#k c #16547b",
".g c #1774a3",
".F c #193e5c",
"#j c #1a577e",
"#c c #205c81",
".c c #207aa6",
"#b c #22638a",
"#d c #245f85",
"#e c #286286",
".M c #287ea9",
".C c #2a4d6b",
"#r c #2a729a",
"#f c #2b6588",
".X c #2c698e",
".h c #2c81ab",
".i c #3184ad",
"#g c #326a8c",
"#h c #386e8e",
"#i c #3a6f90",
".j c #3a89b1",
".7 c #3c86ab",
"#a c #3d7394",
"## c #3d7799",
"#. c #3d7a9d",
".9 c #3d7fa2",
".8 c #3d82a6",
".b c #418eb4",
".6 c #438bb0",
"#D c #4590b5",
"#N c #487d9d",
"#M c #4883a4",
"#L c #4887a9",
"#K c #488aad",
".5 c #488fb3",
".k c #4a93b7",
".H c #5096b9",
".l c #5298bb",
".E c #559abc",
".4 c #5999b9",
".m c #5b9dbe",
".I c #60a0bf",
".n c #62a2c1",
".a c #65a3c2",
".Z c #68a3c0",
".J c #6aa6c4",
"#E c #6ea9c6",
".0 c #71a8c4",
".1 c #73abc7",
".2 c #7dafc8",
"#J c #83a7be",
".# c #85b6ce",
".3 c #8ab7ce",
".W c #95b5c8",
"#H c #96bbcf",
"#F c #96bfd4",
"#I c #97b8cb",
"#G c #97c2d7",
"Qt c #9dc5d9",
".D c #9fc6da",
".G c #a7c9db",
".V c #a8c6d6",
".U c #a8ccde",
".L c #abcedf",
".T c #b4d3e2",
".S c #b7d5e4",
".N c #bdd8e6",
".O c #c1dbe8",
".P c #c4dde9",
".Q c #c9e0eb",
".R c #cbe1eb",
"Qt.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.f.g.c.c.h.i.j.b.k.k.l.m.m.n.n.a.a.n.n.m.m.l.k.k.b.j.i.h.c.c.g.f.e.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.C",
".D.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.f.g.c.c.h.i.j.b.k.k.E.m.m.n.a.a.a.a.n.m.m.E.k.k.b.j.i.h.c.c.g.f.e.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.F",
".G.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.f.g.c.c.h.i.j.b.k.H.E.m.I.n.a.J.J.a.n.I.m.E.H.k.b.j.i.h.c.c.g.f.e.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.K",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.f.g.c.c.h.i.j.b.k.H.E.m.I.n.a.J.J.a.n.I.m.E.H.k.b.j.i.h.c.c.g.f.e.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.A",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.f.g.c.M.h.m.L.N.O.P.P.P.Q.R.R.R.R.R.R.Q.P.P.P.O.N.N.N.S.S.T.L.L.L.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.G.G.G.G.G.G.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.W.X.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.A",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.Y.g.c.M.i.m.Z.J.0.1.2.2.#.#.3.3.3.3.#.#.2.2.1.1.J.Z.I.m.4.H.k.5.6.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.7.7.7.7.7.7.7.7.7.7.8.8.8.8.8.8.8.8.8.9.9.9.9.9.9#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#####a#a#a#a#a.X.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.C",
"Qt.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d.d.e.Y.g.c.M.i#b#c#d#e#f#g#g#g#h#h#i#i#h#h#g#g#g#f#e#d#c#c#j#k#l#m#m.v.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x#n#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o.y.y.y#p#p#p#p#p#p#p#p#p#p#p#o.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.F",
".D.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.Y.g.c.M.i#r#s#t#u#u#u#u#v#w#w#w#w#w#w#v#u#u#u#u#t#x#y#y#z#z#A.z#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.B.B.B.B.B.B.B#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C.A.x.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.K",
".G.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.Y.g.c.M.i.7.b.b.k.l.m.m.n.J.J.J.J.J.J.a.I.m.l.k#D.b.j.i.M.c.g.Y.e#q.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.C",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.Y.g.c.M.i.j.b#D.k.l.m.I.a.J.J#E#E.J.J.a.I.m.l.k#D.b.j.i.M.c.g.Y.e#q.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.F",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.j#F.U.U.L.L.L.T.S.S.N.N.O.P.P.P.P.P.P.O.N.N.S.S.T.L.L.L.U.U.G.DQt#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#G#F#F#F#F#F#F#F#F#F#F#F#F#F#F#F#F#F#F#F#F#H#H#H#H#H#H#H#H#H#H#H#H#H#H#H#H#I#I#I#I#I#I#I#I#I#I#I#I#I#I.W.W#J#f.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.K",
"Qt.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.5.4.m.I.Z.J.1.1.2.#.#.3.3#H#F#F#F#F#H.3.3.#.#.2.1.1.J.Z.I.m.4.H.k.k.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5#K#K#K#K#K#K#K#K#K#K#K#K#K#K#L#L#M#M#M#M#M#M#M#M#M#M#M#M#M#M#M#M#M#M#M#N#N#N#N#N#N#N#N#N#N#N#N#N#N#g.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.A",
".D.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e#O#P#l#k#j#c#c#e#e#f#g#g#h#i#a#a#a#a#i#h#g#g#f#e#e#c#c#j#k#l#P#m#m.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x#n#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o.y.y.y#p#p#p#p#p#p#p#p#p#p#p#p#p.z.z.z.y.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.A",
".G.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e#Q#R#z#y#y#y#t#t#u#u#u#u#w#w#w#w#w#w#w#w#u#u#u#u#u#t#y#y#y#z#A.z.z#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B#B.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.B.B.B.B.B.B.B#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C#C.A.x.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.K",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.Y.g.c.M.i.j.b.k.k.E.m.n.J.J.1.1.1.1.J.J.n.m.E.H.k.b.j.i.M.c.g.Y.e#q.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.A",
".L.#.a.b.c.d.d.d.d.d.d.d.d.d.d.d#q.e.Y.g.c.M.i.j.b.k.H.E.m.n.J#E.1.1.1.1#E.J.n.m.E.H.k.b.j.i.M.c.g#S.e#q.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.o.o.o.o.o.o.o.o.o.o.o.o.o.p.q.q.q.q.q.q.q.q.q.q.q.q.r.s.s.s.s.s.s.s.s.s.s.t.t.t.t.t.u.u.u.u.u.u.u.v.v.v.v.v.v.v.v.v.w.w.w.x.x.x.x.x.x.x.y.z.A.B.A"};

extern const char* description;
extern const char* version;

/**
 * @brief Constructor
 * @param parent parent widget
 * @param name name of widget
 * @param fl window flags
 */
BPMDetectWidget::BPMDetectWidget( QWidget* parent, const char* name, WFlags fl )
    : BPMDetectWidgetBase( parent, name, fl ) {
  stop = TRUE;
  citem = 0;
  Load_Settings();
//  chbSkipScanned->setChecked( !force );
//  chbSave->setChecked( bpmsave );
  chbSkipScanned->setChecked( set_skip );
  chbSave->setChecked( set_save );
  cbFormat->setCurrentText( set_format );
/*
#ifndef HAVE_TAGLIB
  chbSave->setChecked( false );
  chbSave->setEnabled( false );
#endif
*/

  /// Createing TrackList menu
  ListMenu = new QPopupMenu( this );
  QLabel* caption = new QLabel(0);
  caption->setPixmap( topxpm );
  caption->setScaledContents(true);
  ListMenu->insertItem( caption );
  ListMenu->insertItem( "Add files", this, SLOT( addFiles() ) );
  ListMenu->insertItem( "Add directory", this, SLOT( addDir() ) );
  ListMenu->insertSeparator();
  ListMenu->insertItem( "Remove selected tracks", this, SLOT( removeSelected() ) );
  ListMenu->insertItem( "Remove tracks with BPM", this, SLOT( clearDetected() ) );
  ListMenu->insertItem( "Clear track list", this, SLOT( clearTrackList() ) );
  ListMenu->insertSeparator();
  ListMenu->insertItem( "Test BPM", this, SLOT( testBPM() ) );

  TrackList->addColumn("BPM", 60);
  TrackList->addColumn("Artist", 200);
  TrackList->addColumn("Title", 200);
  TrackList->addColumn("Length", 60);
  TrackList->addColumn("Filename", 400);

  setCaption("BPM Detect");
  connect(TrackList, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
    this, SLOT(listMenuPopup( QListViewItem*, const QPoint& )));
  connect(TrackList, SIGNAL(keyPress(QKeyEvent *)),
    this, SLOT(trackListKeyPressed( QKeyEvent *)));
  connect(TrackList, SIGNAL(drop(QDropEvent *)),
    this, SLOT(dropped(QDropEvent *)));
}

/// @brief Destructor
BPMDetectWidget::~BPMDetectWidget() {
  /// Stop detection
  if ( !stop ) start();
  /// Sae settings
  set_skip = chbSkipScanned->isChecked();
  set_save = chbSave->isChecked();
  set_format = cbFormat->currentText();
  Save_Settings();
}

/**
 * @brief Convert miliseconds to time string
 * @param ms miliseconds
 * @return time string
 */
inline QString BPMDetectWidget::msec2time( uint ms ) {
  static SONGTIME sTime;
  QString timestr;
  sTime.msecs = ms / 10;
  sTime.secs = sTime.msecs / 100;
  sTime.msecs = sTime.msecs - ( sTime.secs * 100 );
  sTime.mins = sTime.secs / 60;
  sTime.secs = sTime.secs - ( sTime.mins * 60 );
  QString s;
  timestr.sprintf( "%d:", sTime.mins );
  s.sprintf( "%d.", sTime.secs );
  timestr.append( s.rightJustify( 3, '0' ) );
  s.sprintf( "%d", sTime.msecs );
  timestr.append( s.rightJustify( 2, '0' ) );
  return timestr;
}

void BPMDetectWidget::enableControls(bool e) {
  btnAddFiles->setEnabled( e );
  btnAddDir->setEnabled( e );
  btnRemoveSelected->setEnabled( e );
  btnClearList->setEnabled( e );
  TrackList->setEnabled( e );
  cbFormat->setEnabled( e );

  if(e) {
    btnStart->setText( "Start" );
    lblCurrentTrack->setText("");
    TrackList->setSelectionMode(QListView::Extended);
    TotalProgress->setProgress( 0 );
    CurrentProgress->setProgress( 0 );
  } else {
    btnStart->setText( "Stop" );
    TrackList->setSelectionMode(QListView::Single);
  }

  TrackList->clearSelection();
}

/**
 * @brief Start/Stop detection
 * Detect BPM of all tracks in TrackList
 */
void BPMDetectWidget::start() {
  if ( !stop ) {      // Stop scanning
    stop = TRUE;
    enableControls( true );
  } else {            // Start scanning
    stop = FALSE;
    enableControls( false);

    QListViewItemIterator it( TrackList );
    if ( TrackList->childCount() )
      TotalProgress->setTotalSteps( TrackList->childCount() );

    for ( int progress = 0; it.current(); ++it ) {
      FMOD_SOUND *sound;
      FMOD_RESULT result;
      progress++;

      cout << "Track " << progress << " of " << TrackList->childCount() << endl;
      TotalProgress->setProgress( progress );
      TrackList->ensureItemVisible( it.current() );
      TrackList->setSelected(it.current(), TRUE);

      QString cfile = it.current()->text( TrackList->columns() - 1 );
      cfile = cfile.right( cfile.length() - cfile.findRev( "/" ) - 1 );
      lblCurrentTrack->setText( cfile );
      cout << cfile << endl;

      if( chbSkipScanned->isChecked() &&
          it.current()->text( 0 ).toFloat() > 50. &&
          it.current()->text( 0 ).toFloat() < 250.) {
        Print_BPM(it.current()->text( 0 ).toFloat());
        continue;
      }
      result = FMOD_System_CreateStream( SoundSystem,
                 it.current()->text( 4 ).local8Bit(),
                 FMOD_OPENONLY, 0, &sound );
      if ( result != FMOD_OK ) {
        cerr << FMOD_ErrorString( result ) << " : " <<
        it.current()->text( 4 ).local8Bit() << endl;
        Print_BPM(0);
        continue;
      }

      {
#define CHUNKSIZE 4096
        int16_t data16[ CHUNKSIZE / 2 ];
        int8_t data8[ CHUNKSIZE ];
        SAMPLETYPE samples[ CHUNKSIZE / 2 ];
        unsigned int length = 0, read;
        int channels = 2, bits = 16;
        float frequency = 44100;
        result = FMOD_Sound_GetLength( sound, &length, FMOD_TIMEUNIT_PCMBYTES );
        FMOD_Sound_GetDefaults( sound, &frequency, 0, 0, 0 );
        FMOD_Sound_GetFormat ( sound, 0, 0, &channels, &bits );

        if ( bits != 16 && bits != 8 ) {
          cerr << bits << " bit samples are not supported!" << endl;
          Print_BPM( 0 );
          cout << endl;
          continue;
        }

        BPMDetect bpmd( channels, ( int ) frequency );
        CurrentProgress->setTotalSteps( length / CHUNKSIZE );
        int cprogress = 0;
        do {
          if ( cprogress % 20 == 0 )
            CurrentProgress->setProgress( cprogress );
          if( bits == 16 ) {
            result = FMOD_Sound_ReadData( sound, data16, CHUNKSIZE, &read );
            for ( uint i = 0; i < read / 2; i++ ) {
              samples[ i ] = ( float ) data16[ i ] / 32768;
            }
            bpmd.inputSamples( samples, read / ( 2 * channels ) );
          } else if ( bits == 8 ) {
            result = FMOD_Sound_ReadData( sound, data8, CHUNKSIZE, &read );
            for ( uint i = 0; i < read; i++ ) {
              samples[ i ] = ( float ) data8[ i ] / 128;
            }
            bpmd.inputSamples( samples, read / channels );
          }
          cprogress++;
          if ( cprogress % 25 == 0 )
            qApp->processEvents();
        } while ( result == FMOD_OK && read == CHUNKSIZE && !stop );
        FMOD_Sound_Release(sound); sound = 0;

        if ( !stop ) {
          float BPM = bpmd.getBpm();
          if ( BPM != 0. ) BPM = Correct_BPM( BPM );
          it.current()->setText( 0, BPM2str(BPM) );
          /// Save BPM
          if ( BPM != 0. && chbSave->isChecked() )
            Save_BPM( it.current()->text( TrackList->columns() - 1 ), BPM );
          Print_BPM( BPM );
        }
      }
      lblCurrentTrack->setText( "" );
      if ( stop ) break;
    }
    stop = TRUE;
    enableControls( true );
    cout << "Done" << endl << endl;
  }
}

/**
 * @brief Add files to TrackList
 *
 * @param files is list of file names to add
 */
void BPMDetectWidget::addFiles( QStringList &files ) {
  QStringList::Iterator it = files.begin();
  while ( it != files.end() ) {
    QString bpm, artist, title, length;
    FMOD_SOUND *sound;
    FMOD_TAG tag;
    FMOD_RESULT result;
    unsigned int len;

    result = FMOD_System_CreateStream( SoundSystem,
               (*it).local8Bit(), FMOD_OPENONLY, 0, &sound );
    if ( result != FMOD_OK ) {
      ++it;
      cerr << FMOD_ErrorString( result ) << " : " << ( *it ) << endl;
      continue;
    }
    FMOD_SOUND_TYPE type;
    int bits = 0;
    FMOD_Sound_GetFormat ( sound, &type, 0, 0, &bits );
    if( bits != 16 && bits != 8 ) {
      FMOD_Sound_Release(sound);
      ++it;
      continue;
    }

    FMOD_Sound_GetLength( sound, &len, FMOD_TIMEUNIT_MS );
    length = msec2time( len );

    if( type == FMOD_SOUND_TYPE_WAV) {
      FMOD_Sound_Release(sound); sound = 0;
      TAGINFO tinf = GetTagInfoWAV( *it );
      artist = tinf.Artist;
      title = tinf.Title;
      bpm = tinf.BPM;
    } else if( type == FMOD_SOUND_TYPE_MPEG) {
      FMOD_Sound_Release(sound); sound = 0;
      TAGINFO tinf = GetTagInfoMPEG( *it);
      artist = tinf.Artist;
      title = tinf.Title;
      bpm = tinf.BPM;
    } else {
      if ( FMOD_Sound_GetTag( sound, "TBPM", 0, &tag ) == FMOD_OK ) {
        QString s = ( char* ) tag.data;
        bpm = BPM2str(Str2BPM(s));
      } else bpm = "000.00";
      if ( FMOD_Sound_GetTag( sound, "ARTIST", -1, &tag ) == FMOD_OK )
        artist = ( char* ) tag.data;
      if ( FMOD_Sound_GetTag( sound, "TITLE", -1, &tag ) == FMOD_OK )
        title = ( char* ) tag.data;
      else
        title = (*it).right( (*it).length() - (*it).findRev("/") - 1 );
      FMOD_Sound_Release(sound); sound = 0;
    }

    new QListViewItem( TrackList, bpm, artist, title, length, *it );
    ++it;
    qApp->processEvents();
  }
}

/**
 * @brief Add files to TrackList
 * Open file dialog and add selected files to TrackList
 */
void BPMDetectWidget::addFiles() {
  QStringList files;
  files += QFileDialog::getOpenFileNames(
            "Audio files (*.wav *.mp3 *.ogg *.flac)",
            recentpath, this, "Add tracks", "Select tracks" );
  if(files.count() > 0)
    recentpath = files[0].left(files[0].findRev( "/" ));
  addFiles( files );
}

/**
 * @brief Add directory to TrackList
 * Open file dialog to select path and add files from
 * directory including subdirectories to TrackList
 */
void BPMDetectWidget::addDir() {
  QString path = QFileDialog::getExistingDirectory (
            recentpath, this, 0, "Add directory" );

  if ( path != QString::null ) {
    recentpath = path;
    QStringList list;
    list += filesFromDir( path );
    if ( list.count() == 0 ) return;

    if ( !path.endsWith( "/" ) )
      path.append( "/" );

    QStringList files;
    for ( uint i = 0; i < list.count(); i++ ) {
      QString filename = path + list[ i ];
      files.append( filename );
    }

    addFiles( files );
  }
}

/**
 * @brief Popup Tracklist menu
 * @param item TrackList item
 * @param p popup point
 */
void BPMDetectWidget::listMenuPopup( QListViewItem* item, const QPoint &p ) {
  ListMenu->popup( p );
  citem = item;
}

/**
 * @brief Get all wav, ogg, flac and mp3 files
 * from directory path including files from subdirectories
 * @param path path from which the files are added
 * @return QStringList of files with relative paths
 * to @param path
 */
QStringList BPMDetectWidget::filesFromDir( QString path ) {
  QDir d( path ), f( path ); QStringList files;

  if ( d.exists( path ) ) {
    d.setFilter( QDir::Dirs | QDir::Hidden | QDir::NoSymLinks );
    f.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    f.setNameFilter( "*.wav *.mp3 *.ogg *.flac" );

    QStringList dirs = d.entryList();
    files = f.entryList();

    if ( dirs.count() ) {
      for ( QStringList::Iterator it = dirs.begin() ; it != dirs.end() ; ++it ) {
        if ( ( *it ) != "." && ( *it ) != ".." ) {
          QStringList nfiles = filesFromDir( d.absPath() + "/" + ( *it ) );
          if ( nfiles.count() ) {
            for ( QStringList::Iterator nit = nfiles.begin();
                  nit != nfiles.end(); ++nit ) {
              files.append( *it + "/" + *nit );
            }
          }
        }
      }
    }
    return files;
  }

  return files;
}

/// @brief Remove selected tracks from TrackList
void BPMDetectWidget::removeSelected() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if ( it.current() != TrackList->firstChild() && it.current() ->isSelected() ) {
      TrackList->removeItem( it.current() ); it--;
    }
  }
  if ( TrackList->firstChild() && TrackList->firstChild() ->isSelected() )
    TrackList->removeItem( TrackList->firstChild() );
  
}

/// @brief Show dialog to test detected BPM of current track
void BPMDetectWidget::testBPM() {
  if ( citem != NULL && citem->text( 0 ).toFloat() != 0. ) {
    float bpm = citem->text( 0 ).toFloat();
    TestBPM tbpmd( SoundSystem, citem->text( TrackList->columns() - 1 ), bpm, this );
    int ret;
    ret = tbpmd.exec();
    tbpmd.stop();
  }
}

/// @brief Show about dialog
void BPMDetectWidget::showAbout() {
  QString abouttext = " \
Version:    \t%1 \n \
Description:\t%2 \n \
Author:     \tMartin Sakmar \n \
e-mail:     \tmartin.sakmar@gmail.com \n \
License:    \tGNU General Public License \
";
  abouttext.replace("%1", version);
  abouttext.replace("%2", description);
  QMessageBox::about(this, "About BPM Detect", abouttext);
}


/// @brief Clear the TrackList
void BPMDetectWidget::clearTrackList() {
  TrackList->clear();
}

/// @brief Clear tracks with detected BPM
void BPMDetectWidget::clearDetected() {
  QListViewItemIterator it( TrackList );
  for ( ; it.current(); it++ ) {
    if ( it.current() != TrackList->firstChild() ) {
      float fBPM = it.current()->text(0).toFloat();
      if(fBPM >= 50.) {
        TrackList->removeItem( it.current() );
        it--;
      }
    }
  }
  if ( TrackList->firstChild() ) {
    float fBPM = TrackList->firstChild()->text(0).toFloat();
    if(fBPM >= 50.) TrackList->removeItem( TrackList->firstChild() );
  }
}

void BPMDetectWidget::formatChanged(const QString &f) {
  set_format = f;
}

void BPMDetectWidget::trackListKeyPressed(QKeyEvent *e) {
  if(e->key() == Qt::Key_Delete) removeSelected();
}

void BPMDetectWidget::dropped(QDropEvent* e) {
  e->accept(1);
  QStringList files;
  if ( QUriDrag::decodeLocalFiles( e, files ) )
    addFiles(files);
  return;
}

#include "bpmdetectwidget.moc"

