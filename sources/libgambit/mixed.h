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

#ifndef LIBGAMBIT_MIXED_H
#define LIBGAMBIT_MIXED_H

#include "base.h"
#include "gpvector.h"

namespace Gambit {

template <class T> class MixedStrategyProfile : public gbtPVector<T>  {
private:
  StrategySupport support;

  // Private Payoff functions
  T PPayoff(int pl, int index, int i) const;
  void PPayoff(int pl, int const_pl, int const_st, int cur_pl, long index,
	       T prob, T&value) const;
  void PPayoff(int pl, int const_pl1, int const_st1, int const_pl2, 
	       int const_st2, int cur_pl, long index, T prob, T &value) const;
  void PPayoff(int pl, int const_pl, int cur_pl, long index, T prob,
	       gbtVector<T> &value) const;

public:
  /// @name Lifecycle
  //@{
  MixedStrategyProfile(const StrategySupport &);
  MixedStrategyProfile(const MixedStrategyProfile<T> &);
  MixedStrategyProfile(const MixedBehavProfile<T> &);
  virtual ~MixedStrategyProfile() { }

  MixedStrategyProfile<T> &operator=(const MixedStrategyProfile<T> &);
  //@}

  /// @name Operator overloading
  //@{
  bool operator==(const MixedStrategyProfile<T> &) const;
  bool operator!=(const MixedStrategyProfile<T> &x) const
  { return !(*this == x); }
  //@}

  /// @name General data access
  //@{
  Game GetGame(void) const { return support.GetGame(); }
  const StrategySupport &GetSupport(void) const { return support; }
  void SetCentroid(void);
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetPayoff(int pl) const;
  T GetPayoff(int pl, int player1, int strat1) const;
  T GetPayoff(int pl, const GameStrategy &) const;
  T GetPayoff(int pl, int player1, int strat1, int player2, int strat2) const;

  T GetLiapValue(void) const;
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_MIXED_H


