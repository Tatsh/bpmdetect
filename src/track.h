/***************************************************************************
     Copyright          : (C) 2008 by Martin Sakmar
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

#ifndef TRACK_H
#define TRACK_H

#include <string>

typedef struct taginfo_s {
  std::string BPM;
  std::string Artist;
  std::string Title;
} taginfo_t;

/**
*/
class Track {
public:
  Track();
  ~Track();

  /// Convert std::string to BPM
  static double str2bpm( std::string sBPM );
  /// Convert BPM to std::string using selected format
  static std::string bpm2str( double dBPM, std::string format = "0.00");

protected:
  /// Save BPM to file
  void saveBPM( std::string filename, double dBPM, std::string format = "0.00" );
  /// Correct detected BPM
  double correctBPM( double dBPM, double min = 80., double max = 185. );
  /// Print detected BPM to stdout
  void printBPM( double dBPM, std::string format = "0.00" );
  double detectBPM( std::string filename );

  /// Read ID3v2 tag (mp3 file)
  taginfo_t getTagInfoMPEG(std::string filename);
  /// Read ID3v2 tag (wav file)
  taginfo_t getTagInfoWAV(std::string filename);
};

#endif
