//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to objects representing players in normal form games
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef NFPLAYER_H
#define NFPLAYER_H

class Strategy;

class NFPlayer {
  friend class Nfg;

private:
  int number;
  gText name;
  Nfg *N;
  
  gArray<Strategy *> strategies;

public:
  NFPlayer(int n, Nfg *no, int num);
  ~NFPlayer();

  Nfg &Game(void) const;
  
  const gText &GetName(void) const;
  void SetName(const gText &s);

  int NumStrats(void) const;

  const gArray<Strategy *> &Strategies(void) const;
  inline Strategy *GetStrategy(const int i) { return Strategies()[i]; }

  int GetNumber(void) const  { return number; }
};

#endif    // NFPLAYER_H


