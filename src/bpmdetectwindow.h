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

#ifndef _BPMDETECT_H_
#define _BPMDETECT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif  // HAVE_CONFIG_H

#include <qmainwindow.h>

#include "bpmdetectwidget.h"

/// @short Application Main Window
class BPMDetectWindow : public QMainWindow {
   Q_OBJECT
public:
   /// Constructor
   BPMDetectWindow();
   /// Destructor
   virtual ~BPMDetectWindow();

  /// Add files from list
  void addFiles(QStringList &files);
protected:
  BPMDetectWidget* bpmwid;
};

#endif // _BPMDETECT_H_
