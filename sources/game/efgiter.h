//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to extensive form action iterators
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

#ifndef EFGITER_H
#define EFGITER_H

#include "efg.h"

class EfgContIter;

class EfgIter    {
  private:
    const efgGame *_efg;
    EFSupport _support;
    PureBehavProfile<gNumber> _profile;
    gPVector<int> _current;
    mutable gVector<gNumber> _payoff;

  public:
    EfgIter(efgGame &);
    EfgIter(const EFSupport &);
    EfgIter(const EfgIter &);
    EfgIter(const EfgContIter &);
    ~EfgIter();
  
    EfgIter &operator=(const EfgIter &);
  
    void First(void);
    int Next(int p, int iset);
    int Set(int p, int iset, int act);
  
    gNumber Payoff(int p) const;
    void Payoff(gVector<gNumber> &) const;

    const EFSupport &Support(void) const;

    void Dump(gOutput &) const;
};

#endif   // EFGITER_H




