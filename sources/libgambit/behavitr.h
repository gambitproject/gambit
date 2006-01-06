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

#ifndef LIBGAMBIT_BEHAVITR_H
#define LIBGAMBIT_BEHAVITR_H

#include "behavspt.h"

namespace Gambit {

//
// N.B.: Currently, the contingency iterator only allows one information
//       set to be "frozen".  There is a list of "active" information
//       sets, which are those whose actions are cycled over, the idea
//       being that behavior at inactive information sets is irrelevant.
//
//       Iterating across all contingencies can be achieved by freezing
//       player number 0 (this is the default state on initialization)
//
class BehavIterator {
private:
  int _frozen_pl, _frozen_iset;
  Game _efg;
  BehavSupport _support;
  PureBehavProfile _profile;
  gbtPVector<int> _current;
  gbtArray<gbtArray<bool> > _is_active;
  gbtArray<int> _num_active_infosets;
  mutable gbtVector<Rational> _payoff;

public:
  BehavIterator(const BehavSupport &);
  BehavIterator(const BehavSupport &, 
		const gbtList<GameInfoset> &);
  ~BehavIterator();

  Game GetGame(void) const { return _efg; }
  const BehavSupport &GetSupport(void) const { return _support; }
  
  void First(void);
  
  void Freeze(int pl, int iset);
  
  // These next two only work on frozen infosets
  void Set(int pl, int iset, int act);
  void Set(const GameAction &a);
  int Next(int pl, int iset);
  
  const PureBehavProfile &GetProfile(void) const  { return _profile; }

  int NextContingency(void);
  
  Rational GetPayoff(int pl) const;
};

} // end namespace Gambit

#endif // LIBGAMBIT_BEHAVITR_H




