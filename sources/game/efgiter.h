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

#include "game.h"

class gbtEfgContIterator;

class gbtEfgIterator    {
  private:
    gbtGame m_efg;
    gbtPureBehavProfile _profile;
    gbtPVector<int> _current;
    mutable gbtVector<gbtRational> _payoff;

  public:
    gbtEfgIterator(gbtGame);
    gbtEfgIterator(const gbtEfgIterator &);
    gbtEfgIterator(const gbtEfgContIterator &);
    ~gbtEfgIterator();
  
    gbtEfgIterator &operator=(const gbtEfgIterator &);
  
    void First(void);
    int Next(int p, int iset);
    int Set(int p, int iset, int act);
  
    gbtRational Payoff(int p) const;
    void Payoff(gbtVector<gbtRational> &) const;

    void Dump(std::ostream &) const;
};

#endif   // EFGITER_H
