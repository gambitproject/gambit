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

#include "pvector.h"

namespace Gambit {

/// \brief A probability distribtion over strategies in a game
///
/// A probability distribution over strategies, such that each player
/// independently chooses from among his strategies with specified
/// probabilities.
template <class T> class MixedStrategyProfile : public PVector<T>  {
private:
  StrategySupport support;

  /// @name Private recursive payoff functions
  //@{
  /// Recursive computation of payoff to player pl
  T GetPayoff(int pl, int index, int i) const;
  /// Recursive computation of payoff derivative
  void GetPayoffDeriv(int pl, int const_pl, int cur_pl, long index,
		      const T &prob, T &value) const;
  /// Recursive computation of payoff second derivative
  void GetPayoffDeriv(int pl, int const_pl1, int const_pl2, 
		      int cur_pl, long index, const T &prob, T &value) const;
  //@}

public:
  /// @name Lifecycle
  //@{
  /// Construct a mixed strategy profile at the centroid on the support
  MixedStrategyProfile(const StrategySupport &);
  /// Copy a mixed strategy profile
  MixedStrategyProfile(const MixedStrategyProfile<T> &);
  /// Convert a behavior strategy profile to a mixed strategy profile
  MixedStrategyProfile(const MixedBehavProfile<T> &);

  /// Copy a mixed strategy profile
  MixedStrategyProfile<T> &operator=(const MixedStrategyProfile<T> &);
  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two profiles
  bool operator==(const MixedStrategyProfile<T> &p_profile) const
  { return (support == p_profile.support &&
	    PVector<T>::operator==(p_profile)); }
  /// Test for the inequality of two profiles
  bool operator!=(const MixedStrategyProfile<T> &p_profile) const
  { return (support != p_profile.support ||
	    PVector<T>::operator!=(p_profile)); }

  /// Partitioned access to strategy probability (deprecated)
  const T &operator()(int pl, int st) const
    { return PVector<T>::operator()(pl, st); }
  /// Partitioned access to strategy probability (deprecated)
  T &operator()(int pl, int st)
    { return PVector<T>::operator()(pl, st); }

  /// Returns the probability the strategy is played
  const T &operator()(const GameStrategy &p_strategy) const
    { return (*this)(p_strategy->GetPlayer()->GetNumber(),
		     support.GetIndex(p_strategy)); }
  /// Returns the probability the strategy is played
  T &operator()(const GameStrategy &p_strategy)
    { return (*this)(p_strategy->GetPlayer()->GetNumber(),
		     support.GetIndex(p_strategy)); }
  //@}

  /// @name General data access
  //@{
  /// Returns the game on which the profile is defined
  Game GetGame(void) const { return support.GetGame(); }
  /// Returns the support on which the profile is defined
  const StrategySupport &GetSupport(void) const { return support; }

  /// Sets all strategies for each player to equal probabilities
  void SetCentroid(void);

  /// Returns the total number of strategies in the profile
  int MixedProfileLength(void) const { return Array<T>::Length(); }

  /// Converts the profile to one on the full support of the game
  MixedStrategyProfile<T> ToFullSupport(void) const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  /// Computes the payoff of the profile to player 'pl'
  T GetPayoff(int pl) const;

  /// Computes the payoff of the profile to the player
  T GetPayoff(const GamePlayer &p_player) const
  { return GetPayoff(p_player->GetNumber()); }

  /// \brief Computes the derivative of the player's payoff
  /// 
  /// Computes the derivative of the payoff to the player with respect
  /// to the probability the strategy is played
  T GetPayoffDeriv(int pl, const GameStrategy &) const;
  
  /// \brief Computes the second derivative of the player's payoff
  ///
  /// Computes the second derivative of the payoff to the player,
  /// with respect to the probabilities with which the strategies are played
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const;

  /// Computes the payoff to playing the pure strategy against the profile
  T GetStrategyValue(const GameStrategy &p_strategy) const
  { return GetPayoffDeriv(p_strategy->GetPlayer()->GetNumber(), p_strategy); }

  /// \brief Computes the Lyapunov value of the profile
  ///
  /// Computes the Lyapunov value of the profile.  This is a nonnegative
  /// value which is zero exactly at Nash equilibria.  This version
  /// implements a positive penalty for profiles which are not on the
  /// simplotope (useful for penalty-function minimization methods).
  T GetLiapValue(void) const;
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_MIXED_H


