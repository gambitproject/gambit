//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to objects representing players in extensive forms
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

#ifndef EFPLAYER_H
#define EFPLAYER_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class EFPlayer   {
  friend class efgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  private:
    int number;
    gText name;
    efgGame *E;
    
    gBlock<Infoset *> infosets;

    EFPlayer(efgGame *e, int n) : number(n), E(e)  { }
    ~EFPlayer();

  public:
    efgGame *Game(void) const   { return E; }
  
    const gText &GetName(void) const   { return name; }
    void SetName(const gText &s)       { name = s; }

    bool IsChance(void) const      { return (number == 0); }

    int NumInfosets(void) const    { return infosets.Length(); }
    const gArray<Infoset *> &Infosets(void) const  { return infosets; }
    const Infoset *GetInfoset(const int iset) const { return infosets[iset]; }

    int GetNumber(void) const    { return number; }
};


#endif    // EFPLAYER_H


