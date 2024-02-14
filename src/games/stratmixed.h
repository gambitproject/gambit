//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
  unsigned int m_gameversion;

  explicit MixedStrategyProfileRep(const StrategySupportProfile &);
  virtual ~MixedStrategyProfileRep() = default;
  virtual MixedStrategyProfileRep<T> *Copy() const = 0;

  void SetCentroid();
  MixedStrategyProfileRep<T> *Normalize() const;
  /// Returns the probability the strategy is played
  const T &operator[](const GameStrategy &p_strategy) const
  {
    return m_probs[m_support.m_profileIndex[p_strategy->GetId()]];
  }
  /// Returns the probability the strategy is played
  T &operator[](const GameStrategy &p_strategy)
  {
    return m_probs[m_support.m_profileIndex[p_strategy->GetId()]];
  }

  virtual T GetPayoff(int pl) const = 0;
  virtual T GetPayoffDeriv(int pl, const GameStrategy &) const = 0;
  virtual T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const = 0;

  T GetPayoff(const GamePlayer &p_player) const { return GetPayoff(p_player->GetNumber()); }
  T GetPayoff(const GameStrategy &p_strategy) const
  {
    return GetPayoffDeriv(p_strategy->GetPlayer()->GetNumber(), p_strategy);
  }

  T GetRegret(const GameStrategy &) const;
  T GetRegret(const GamePlayer &) const;
  T GetMaxRegret() const;

  virtual void InvalidateCache() const {};
};

/// \brief A probability distribution over strategies in a game
///
/// A probability distribution over strategies, such that each player
/// independently chooses from among his strategies with specified
/// probabilities.
template <class T> class MixedStrategyProfile {
private:
  MixedStrategyProfileRep<T> *m_rep;

public:
  /// @name Lifecycle
  //@{
  explicit MixedStrategyProfile(MixedStrategyProfileRep<T> *p_rep) : m_rep(p_rep) {}
  /// Convert a behavior strategy profile to a mixed strategy profile
  explicit MixedStrategyProfile(const MixedBehaviorProfile<T> &);

  /// Check underlying game has not changed; raise exception if it has
  void CheckVersion() const
  {
    if (IsInvalidated()) {
      throw GameStructureChangedException();
    }
  }

public:
  /// @name Lifecycle
  //@{
  /// Make a copy of the mixed strategy profile
  MixedStrategyProfile(const MixedStrategyProfile<T> &);
  /// Destructor
  virtual ~MixedStrategyProfile();

  MixedStrategyProfile<T> &operator=(const MixedStrategyProfile<T> &);
  MixedStrategyProfile<T> &operator=(const Vector<T> &v)
  {
    InvalidateCache();
    m_rep->m_probs = v;
    return *this;
  }
  MixedStrategyProfile<T> &operator=(const T &c)
  {
    InvalidateCache();
    m_rep->m_probs = c;
    return *this;
  }
  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two profiles
  bool operator==(const MixedStrategyProfile<T> &p_profile) const
  {
    return (m_rep->m_support == p_profile.m_rep->m_support &&
            m_rep->m_probs == p_profile.m_rep->m_probs);
  }
  /// Test for the inequality of two profiles
  bool operator!=(const MixedStrategyProfile<T> &p_profile) const
  {
    return (m_rep->m_support != p_profile.m_rep->m_support ||
            m_rep->m_probs != p_profile.m_rep->m_probs);
  }

  /// Vector-style access to probabilities
  const T &operator[](int i) const
  {
    CheckVersion();
    return m_rep->m_probs[i];
  }
  /// Vector-style access to probabilities
  T &operator[](int i)
  {
    CheckVersion();
    InvalidateCache();
    return m_rep->m_probs[i];
  }

  /// Returns the probability the strategy is played
  const T &operator[](const GameStrategy &p_strategy) const
  {
    CheckVersion();
    return m_rep->operator[](p_strategy);
  }
  /// Returns the probability the strategy is played
  T &operator[](const GameStrategy &p_strategy)
  {
    CheckVersion();
    InvalidateCache(); // NEW
    return m_rep->operator[](p_strategy);
  }

  /// Returns the mixed strategy for the player
  Vector<T> operator[](const GamePlayer &p_player) const;

  explicit operator const Vector<T> &() const
  {
    CheckVersion();
    return m_rep->m_probs;
  }
  //@}

  /// @name General data access
  //@{
  /// Returns the game on which the profile is defined
  Game GetGame() const { return m_rep->m_support.GetGame(); }
  /// Returns the support on which the profile is defined
  const StrategySupportProfile &GetSupport() const
  {
    CheckVersion();
    return m_rep->m_support;
  }
  /// Returns whether the profile has been invalidated by a subsequent revision to the game
  bool IsInvalidated() const
  {
    return m_rep->m_gameversion != m_rep->m_support.GetGame()->GetVersion();
  }

  /// Sets all strategies for each player to equal probabilities
  void SetCentroid()
  {
    CheckVersion();
    m_rep->SetCentroid();
  }

  /// Create a new mixed strategy profile where strategies are played
  /// in the same proportions, but with probabilities for each player
  /// summing to one.
  MixedStrategyProfile<T> Normalize() const
  {
    CheckVersion();
    return MixedStrategyProfile<T>(m_rep->Normalize());
  }

  /// Returns the total number of strategies in the profile
  size_t MixedProfileLength() const { return m_rep->m_probs.size(); }

  /// Converts the profile to one on the full support of the game
  MixedStrategyProfile<T> ToFullSupport() const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  /// Used to read payoffs from cache or compute them and cache them if needed
  void ComputePayoffs() const;
  mutable std::map<GamePlayer, std::map<GameStrategy, T>> map_strategy_payoffs;
  mutable std::map<GamePlayer, T> map_profile_payoffs;
  /// Reset cache for payoffs and strategy values
  virtual void InvalidateCache() const
  {
    map_strategy_payoffs.clear();
    map_profile_payoffs.clear();
    m_rep->InvalidateCache();
  }

  /// Computes the payoff of the profile to player 'pl'
  T GetPayoff(int pl) const
  {
    CheckVersion();
    return m_rep->GetPayoff(pl);
  }

  /// Computes the payoff of the profile to the player
  T GetPayoff(const GamePlayer &p_player) const
  {
    CheckVersion();
    return GetPayoff(p_player->GetNumber());
  }

  /// \brief Computes the derivative of the player's payoff
  ///
  /// Computes the derivative of the payoff to the player with respect
  /// to the probability the strategy is played
  T GetPayoffDeriv(int pl, const GameStrategy &s) const
  {
    CheckVersion();
    return m_rep->GetPayoffDeriv(pl, s);
  }

  /// \brief Computes the second derivative of the player's payoff
  ///
  /// Computes the second derivative of the payoff to the player,
  /// with respect to the probabilities with which the strategies are played
  T GetPayoffDeriv(int pl, const GameStrategy &s1, const GameStrategy &s2) const
  {
    CheckVersion();
    return m_rep->GetPayoffDeriv(pl, s1, s2);
  }

  /// Computes the payoff to playing the pure strategy against the profile
  T GetPayoff(const GameStrategy &p_strategy) const
  {
    CheckVersion();
    return GetPayoffDeriv(p_strategy->GetPlayer()->GetNumber(), p_strategy);
  }

  /// @brief Computes the regret to playing \p p_strategy
  /// @details Computes the regret to the player of playing strategy \p p_strategy
  ///          against the profile.  The regret is defined as the difference
  ///          between the best-response payoff and the payoff to playing
  ///          \p p_strategy.
  /// @param[in] p_strategy  The strategy to compute the regret for.
  /// @sa GetRegret(const GamePlayer &) const;
  ///     GetMaxRegret() const
  T GetRegret(const GameStrategy &p_strategy) const
  {
    CheckVersion();
    return m_rep->GetRegret(p_strategy);
  }

  /// @brief Computes the regret for player \p p_player
  /// @details Computes the regret to the player of playing their mixed strategy
  ///          as specified in the profile.  The regret is defined as the difference
  ///          between the player's best-response payoff and the payoff to playing
  ///          their specified mixed strategy.
  /// @param[in] p_player  The player to compute the regret for.
  /// @sa GetRegret(const GameStrategy &) const;
  ///     GetMaxRegret() const
  T GetRegret(const GamePlayer &p_player) const
  {
    CheckVersion();
    return m_rep->GetRegret(p_player);
  }

  /// @brief Computes the maximum regret to any player in the profile
  /// @details Computes the maximum of the regrets of the players in the profile.
  /// @sa GetRegret(const GamePlayer &) const;
  ///     GetRegret(const GameStrategy &) const
  T GetMaxRegret() const
  {
    CheckVersion();
    return m_rep->GetMaxRegret();
  }

  /// @brief Computes the Lyapunov value of the profile
  /// @details Computes the Lyapunov value of the profile.  This is a nonnegative
  ///          value which is zero exactly at Nash equilibria.  This version
  ///          implements a positive penalty for profiles which are not on the
  ///          simplotope (useful for penalty-function minimization methods).
  T GetLiapValue() const;
  //@}
};

template <class Generator>
MixedStrategyProfile<double> GameRep::NewRandomStrategyProfile(Generator &generator) const
{
  auto profile = NewMixedStrategyProfile(0.0);
  std::exponential_distribution<> dist(1);
  for (auto player : GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      profile[strategy] = dist(generator);
    }
  }
  return profile.Normalize();
}

template <class Generator>
MixedStrategyProfile<Rational> GameRep::NewRandomStrategyProfile(int p_denom,
                                                                 Generator &generator) const
{
  auto profile = NewMixedStrategyProfile(Rational(0));
  for (auto player : GetPlayers()) {
    std::list<Rational> dist = UniformOnSimplex(p_denom, player->NumStrategies(), generator);
    auto prob = dist.cbegin();
    for (auto strategy : player->GetStrategies()) {
      profile[strategy] = *prob;
      prob++;
    }
  }
  return profile;
}

} // end namespace Gambit

#endif // LIBGAMBIT_MIXED_H
