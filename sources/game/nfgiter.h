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

template <class T> class gArray;

class StrategyProfile;
#include "nfstrat.h"
#include "outcome.h"
class NFSupport;
class NfgContIter;

//
// This class is useful for iterating around the normal form.
//

class NfgIter    {
private:
  NFSupport support;
  Nfg *N;
  gArray<int> current_strat;
  StrategyProfile profile;

public:
  NfgIter(Nfg &);
  NfgIter(const NFSupport &s);
  NfgIter(const NfgIter &);
  NfgIter(const NfgContIter &);
  ~NfgIter();

  NfgIter &operator=(const NfgIter &);

  void First(void);
  int Next(int p);
  int Set(int p, int s);

  void Get(gArray<int> &t) const;
  void Set(const gArray<int> &t);

  long GetIndex(void) const;

  gbtNfgOutcome GetOutcome(void) const;
  void SetOutcome(gbtNfgOutcome);

  const NFSupport &Support(void) const { return support; }
};

#endif   // NFGITER_H




