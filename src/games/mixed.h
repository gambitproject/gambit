//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/mixed.h
// Declaration of mixed strategy profile classes
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

#include "core/vector.h"
#include "games/gameagg.h"
#include "games/gamebagg.h"

namespace Gambit {

template <class T> class MixedStrategyProfileRep {
public:
  Vector<T> m_probs;
  StrategySupportProfile m_support;

  explicit MixedStrategyProfileRep(const StrategySupportProfile &);
  virtual ~MixedStrategyProfileRep() = default;
  virtual MixedStrategyProfileRep<T> *Copy() const = 0;

  void SetCentroid();
  MixedStrategyProfileRep<T> *Normalize() const;
  void Randomize();
  void Randomize(int p_denom);
 /// Returns the probability the strategy is played
  const T &operator[](const GameStrategy &p_strategy) const
    { return m_probs[m_support.m_profileIndex[p_strategy->GetId()]]; }
  /// Returns the probability the strategy is played
  T &operator[](const GameStrategy &p_strategy)
    { return m_probs[m_support.m_profileIndex[p_strategy->GetId()]]; }
  
  virtual T GetPayoff(int pl) const = 0;
  virtual T GetPayoffDeriv(int pl, const GameStrategy &) const = 0;
  virtual T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const = 0;
};

template <class T> class TreeMixedStrategyProfileRep 
  : public MixedStrategyProfileRep<T> {
public:
  explicit TreeMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  { }
  explicit TreeMixedStrategyProfileRep(const MixedBehaviorProfile<T> &);
  ~TreeMixedStrategyProfileRep() override = default;
  
  MixedStrategyProfileRep<T> *Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T> class TableMixedStrategyProfileRep
  : public MixedStrategyProfileRep<T> {
private:
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
  explicit TableMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  { }
  ~TableMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T> class AggMixedStrategyProfileRep
  : public MixedStrategyProfileRep<T> {

public:
  explicit AggMixedStrategyProfileRep(const StrategySupportProfile &p_support)
   : MixedStrategyProfileRep<T>(p_support)
    { }
  ~AggMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override {
    return new AggMixedStrategyProfileRep(*this);
  }
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T> class BagentMixedStrategyProfileRep
  : public MixedStrategyProfileRep<T> {

public:
  explicit BagentMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
    { }
  ~BagentMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override {
    return new BagentMixedStrategyProfileRep(*this);
  }
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

/// \brief A probability distribution over strategies in a game
///
/// A probability distribution over strategies, such that each player
/// independently chooses from among his strategies with specified
/// probabilities.
template <class T> class MixedStrategyProfile {
  friend class StrategySupportProfile;
  friend class MixedStrategyProfileRep<T>;
  friend class TreeMixedStrategyProfileRep<T>;
  friend class AggMixedStrategyProfileRep<T>;
  friend class BagentMixedStrategyProfileRep<T>;
  friend class TableMixedStrategyProfileRep<T>;
  friend class GameAggRep;
  friend class GameBagentRep;
  friend class GameTableRep;
  friend class GameTreeRep;
  friend class MixedBehaviorProfile<T>;
private:
  MixedStrategyProfileRep<T> *m_rep;

  explicit MixedStrategyProfile(MixedStrategyProfileRep<T> *p_rep)
    : m_rep(p_rep)
  { }
  /// Convert a behavior strategy profile to a mixed strategy profile
  explicit MixedStrategyProfile(const MixedBehaviorProfile<T> &);

public:
  /// @name Lifecycle
  //@{
  /// Make a copy of the mixed strategy profile
  MixedStrategyProfile(const MixedStrategyProfile<T> &);
  /// Destructor
  virtual ~MixedStrategyProfile();

  MixedStrategyProfile<T> &operator=(const MixedStrategyProfile<T> &);
  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two profiles
  bool operator==(const MixedStrategyProfile<T> &p_profile) const
  { return (m_rep->m_support == p_profile.m_rep->m_support &&
	    m_rep->m_probs == p_profile.m_rep->m_probs); }
  /// Test for the inequality of two profiles
  bool operator!=(const MixedStrategyProfile<T> &p_profile) const
  { return (m_rep->m_support != p_profile.m_rep->m_support ||
	    m_rep->m_probs != p_profile.m_rep->m_probs); }

  /// Vector-style access to probabilities
  const T &operator[](int i) const { return m_rep->m_probs[i]; }
  /// Vector-style access to probabilities
  T &operator[](int i)             { return m_rep->m_probs[i]; }

  /// Returns the probability the strategy is played
  const T &operator[](const GameStrategy &p_strategy) const
    { return m_rep->operator[](p_strategy); }
  /// Returns the probability the strategy is played
  T &operator[](const GameStrategy &p_strategy)
    { return m_rep->operator[](p_strategy); }

  /// Returns the mixed strategy for the player
  Vector<T> operator[](const GamePlayer &p_player) const;

  explicit operator const Vector<T> &() const { return m_rep->m_probs; }
  explicit operator Vector<T> &() { return m_rep->m_probs; }
  //@}

  /// @name General data access
  //@{
  /// Returns the game on which the profile is defined
  Game GetGame() const { return m_rep->m_support.GetGame(); }
  /// Returns the support on which the profile is defined
  const StrategySupportProfile &GetSupport() const { return m_rep->m_support; }

  /// Sets all strategies for each player to equal probabilities
  void SetCentroid() { m_rep->SetCentroid(); }

  /// Create a new mixed strategy profile where strategies are played
  /// in the same proportions, but with probabilities for each player
  /// summing to one.
  MixedStrategyProfile<T> Normalize() const { return MixedStrategyProfile<T>(m_rep->Normalize()); }

  /// Generate a random mixed strategy profile according to the uniform distribution
  void Randomize() { m_rep->Randomize(); }

  /// Generate a random mixed strategy profile according to the uniform distribution
  /// on a grid with spacing p_denom
  void Randomize(int p_denom) { m_rep->Randomize(p_denom); }

  /// Returns the total number of strategies in the profile
  int MixedProfileLength() const { return m_rep->m_probs.size(); }

  /// Converts the profile to one on the full support of the game
  MixedStrategyProfile<T> ToFullSupport() const;

  /// Converts the profile to one on the unrestricted parent of the game
  MixedStrategyProfile<T> Unrestrict() const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  /// Computes the payoff of the profile to player 'pl'
  T GetPayoff(int pl) const { return m_rep->GetPayoff(pl); }

  /// Computes the payoff of the profile to the player
  T GetPayoff(const GamePlayer &p_player) const
  { return GetPayoff(p_player->GetNumber()); }

  /// \brief Computes the derivative of the player's payoff
  /// 
  /// Computes the derivative of the payoff to the player with respect
  /// to the probability the strategy is played
  T GetPayoffDeriv(int pl, const GameStrategy &s) const
  { return m_rep->GetPayoffDeriv(pl, s); }
  
  /// \brief Computes the second derivative of the player's payoff
  ///
  /// Computes the second derivative of the payoff to the player,
  /// with respect to the probabilities with which the strategies are played
  T GetPayoffDeriv(int pl, const GameStrategy &s1, const GameStrategy &s2) const
  { return m_rep->GetPayoffDeriv(pl, s1, s2); }

  /// Computes the payoff to playing the pure strategy against the profile
  T GetPayoff(const GameStrategy &p_strategy) const
  { return GetPayoffDeriv(p_strategy->GetPlayer()->GetNumber(), p_strategy); }

  /// \brief Computes the Lyapunov value of the profile
  ///
  /// Computes the Lyapunov value of the profile.  This is a nonnegative
  /// value which is zero exactly at Nash equilibria.  This version
  /// implements a positive penalty for profiles which are not on the
  /// simplotope (useful for penalty-function minimization methods).
  T GetLiapValue() const;
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_MIXED_H


