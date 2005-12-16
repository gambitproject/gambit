//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed strategy profile classes
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

#ifndef MIXED_H
#define MIXED_H

#include "base.h"
#include "gpvector.h"
#include "nfstrat.h"


class gbtNfgGame;

template <class T> class gbtRectArray;
template <class T> class gbtBehavProfile;

template <class T> class gbtMixedProfile : public gbtPVector<T>  {
private:
  gbtNfgSupport support;

  // Private Payoff functions
  T PPayoff(int pl, int index, int i) const;
  void PPayoff(int pl, int const_pl, int const_st, int cur_pl, long index,
	       T prob, T&value) const;
  void PPayoff(int pl, int const_pl1, int const_st1, int const_pl2, 
	       int const_st2, int cur_pl, long index, T prob, T &value) const;
  void PPayoff(int pl, int const_pl, int cur_pl, long index, T prob,
	       gbtVector<T> &value) const;
  T Payoff(gbtNfgOutcomeRep *o, int pl) const;

public:
  gbtMixedProfile(const gbtNfgSupport &);
  gbtMixedProfile(const gbtMixedProfile<T> &);
  gbtMixedProfile(const gbtBehavProfile<T> &);
  virtual ~gbtMixedProfile() { }

  gbtMixedProfile<T> &operator=(const gbtMixedProfile<T> &);

  bool operator==(const gbtMixedProfile<T> &) const;

  gbtNfgGame *GetGame(void) const  { return support.GetGame(); }
  const gbtNfgSupport &GetSupport(void) const  { return support; }

  T GetLiapValue(void) const;
  void GetRegret(gbtPVector<T> &value) const;
  T GetMaxRegret(void) const;
  void SetCentroid(void);

  T GetPayoff(int pl) const;
  T GetPayoff(int pl, int player1, int strat1) const;
  T GetPayoff(int pl, const gbtNfgStrategy &) const;
  T GetPayoff(int pl, int player1, int strat1, int player2, int strat2) const;
  void GetPayoff(int pl, int const_pl, gbtVector<T> &payoff) const;
};

#endif    // MIXED_H


