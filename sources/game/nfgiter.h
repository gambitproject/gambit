//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Normal form iterator class
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

#ifndef NFGITER_H
#define NFGITER_H

template <class T> class gbtArray;

#include "nfgcont.h"
#include "nfgsupport.h"

class gbtNfgContIterator;


//
// This is a generic class useful for iterating around the normal form.
// It depends only on the gbtNfgGame interface, so it can be used with
// any implementation providing that interface.
//
class gbtNfgIterator    {
private:
  gbtNfgGame m_nfg;
  gbtArray<int> m_current;
  gbtNfgContingency m_profile;

public:
  gbtNfgIterator(const gbtNfgGame &);
  gbtNfgIterator(const gbtNfgIterator &);
  gbtNfgIterator(const gbtNfgContIterator &);
  ~gbtNfgIterator();

  void First(void);
  int Next(int p);
  int Set(int p, int s);

  void Get(gbtArray<int> &t) const;
  void Set(const gbtArray<int> &t);

  gbtGameOutcome GetOutcome(void) const;

  gbtNumber GetPayoff(const gbtGamePlayer &p_player) const 
    { return m_profile->GetPayoff(p_player); }
};

#endif   // NFGITER_H
