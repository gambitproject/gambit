//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/behav.h
// Behavior strategy profile classes
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

#ifndef LIBGAMBIT_BEHAV_H
#define LIBGAMBIT_BEHAV_H

#include "game.h"

namespace Gambit {

///
/// MixedBehaviorProfile<T> implements a randomized behavior profile on
/// an extensive game.
///
template <class T> class MixedBehaviorProfile {
protected:
  DVector<T> m_probs;
  BehaviorSupportProfile m_support;
  unsigned int m_gameversion;

  // structures for storing cached data: nodes
  mutable std::map<GameNode, T> map_realizProbs, map_beliefs;
  mutable std::map<GameNode, std::map<GamePlayer, T>> map_nodeValues;

  // structures for storing cached data: information sets
  mutable std::map<GameInfoset, T> map_infosetValues;

  // structures for storing cached data: actions
  mutable std::map<GameAction, T> map_actionValues; // aka conditional payoffs
  mutable std::map<GameAction, T> map_regret;

  /// @name Auxiliary functions for computation of interesting values
  //@{
  void GetPayoff(const GameNode &, const T &, const GamePlayer &, T &) const;
  void ComputePass1_realizProbs(const GameNode &node) const;
  void ComputePass2_beliefs_nodeValues_actionValues(const GameNode &node) const;
  void ComputePass3_infosetValues_regret() const;
  void ComputeSolutionData() const;
  //@}

  /// @name Converting mixed strategies to behavior
  //@{
  void BehaviorStrat(GamePlayer &, GameNode &, std::map<GameNode, T> &, std::map<GameNode, T> &);
  void RealizationProbs(const MixedStrategyProfile<T> &, GamePlayer &, const Array<int> &,
                        GameTreeNodeRep *, std::map<GameNode, T> &, std::map<GameNode, T> &);
  //@}

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
  explicit MixedBehaviorProfile(const Game &);
  explicit MixedBehaviorProfile(const BehaviorSupportProfile &);
  MixedBehaviorProfile(const MixedBehaviorProfile<T> &) = default;
  explicit MixedBehaviorProfile(const MixedStrategyProfile<T> &);
  ~MixedBehaviorProfile() = default;

  MixedBehaviorProfile<T> &operator=(const MixedBehaviorProfile<T> &);
  MixedBehaviorProfile<T> &operator=(const Vector<T> &p)
  {
    InvalidateCache();
    m_probs = p;
    return *this;
  }
  MixedBehaviorProfile<T> &operator=(const T &x)
  {
    InvalidateCache();
    m_probs = x;
    return *this;
  }

  //@}

  /// @name Operator overloading
  //@{
  bool operator==(const MixedBehaviorProfile<T> &) const;
  bool operator!=(const MixedBehaviorProfile<T> &x) const { return !(*this == x); }

  bool operator==(const DVector<T> &x) const { return m_probs == x; }
  bool operator!=(const DVector<T> &x) const { return m_probs != x; }

  const T &operator[](const GameAction &p_action) const
  {
    return m_probs(p_action->GetInfoset()->GetPlayer()->GetNumber(),
                   p_action->GetInfoset()->GetNumber(), m_support.GetIndex(p_action));
  }
  T &operator[](const GameAction &p_action)
  {
    InvalidateCache();
    return m_probs(p_action->GetInfoset()->GetPlayer()->GetNumber(),
                   p_action->GetInfoset()->GetNumber(), m_support.GetIndex(p_action));
  }

  const T &operator[](int a) const { return m_probs[a]; }
  T &operator[](int a)
  {
    InvalidateCache();
    return m_probs[a];
  }

  operator const Vector<T> &() const { return m_probs; }
  //@}

  /// @name Initialization, validation
  //@{
  /// Force recomputation of stored quantities
  /// The validity of all caches is determined by the existence of the root node in the
  /// primary cache (first to be computed) map_realizProbs
  /// We also clear
  /// map_nodeValues, map_actionValues
  /// as otherwise we would need to reset them to 0 while populating them
  void InvalidateCache() const
  {
    map_realizProbs.clear();
    map_nodeValues.clear();
    map_actionValues.clear();
  }
  /// Reset certain cached values

  /// Set the profile to the centroid
  void SetCentroid();
  /// Set the behavior at any undefined information set to the centroid
  void UndefinedToCentroid();
  /// Create a new behaviour strategy profile where strategies are played
  /// in the same proportions, but with probabilities for each player
  /// summing to one.
  MixedBehaviorProfile<T> Normalize() const;
  //@}

  /// @name General data access
  //@{
  size_t BehaviorProfileLength() const { return m_probs.Length(); }
  Game GetGame() const { return m_support.GetGame(); }
  const BehaviorSupportProfile &GetSupport() const { return m_support; }
  /// Returns whether the profile has been invalidated by a subsequent revision to the game
  bool IsInvalidated() const { return m_gameversion != m_support.GetGame()->GetVersion(); }

  bool IsDefinedAt(GameInfoset p_infoset) const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetPayoff(int p_player) const;
  T GetPayoff(const GamePlayer &p_player) const { return GetPayoff(p_player->GetNumber()); }
  T GetLiapValue() const;

  const T &GetRealizProb(const GameNode &node) const;
  T GetInfosetProb(const GameInfoset &iset) const;
  const T &GetBeliefProb(const GameNode &node) const;
  Vector<T> GetPayoff(const GameNode &node) const;
  const T &GetPayoff(const GamePlayer &player, const GameNode &node) const;
  const T &GetPayoff(const GameInfoset &iset) const;
  const T &GetPayoff(const GameAction &act) const;
  T GetActionProb(const GameAction &act) const;

  /// @brief Computes the regret to playing \p p_action
  /// @details Computes the regret to the player of playing action \p p_action
  ///          against the profile.  The regret is defined as the difference
  ///          between the best-response payoff and the payoff to playing
  ///          \p p_action.
  /// @param[in] p_action  The action to compute the regret for.
  /// @sa GetRegret(const GameInfoset &) const;
  ///     GetMaxRegret() const
  const T &GetRegret(const GameAction &p_action) const;

  /// @brief Computes the regret at information set \p p_infoset
  /// @details Computes the regret at the information set to the player of playing
  ///          their mixed action as specified in the profile.  The regret is defined
  ///          as the difference between the payoff of the best response action and
  ///          the payoff to playing their specified mixed action.
  /// @param[in] p_infoset  The information set to compute the regret at.
  /// @sa GetRegret(const GameAction &) const;
  ///     GetMaxRegret() const
  T GetRegret(const GameInfoset &p_infoset) const;

  /// @brief Computes the maximum regret at any information set in the profile
  /// @details Computes the maximum of the regrets of the information sets in the profile.
  /// @sa GetRegret(const GameInfoset &) const;
  ///     GetRegret(const GameAction &) const
  T GetMaxRegret() const;

  T DiffActionValue(const GameAction &action, const GameAction &oppAction) const;
  T DiffRealizProb(const GameNode &node, const GameAction &oppAction) const;
  T DiffNodeValue(const GameNode &node, const GamePlayer &player,
                  const GameAction &oppAction) const;

  MixedStrategyProfile<T> ToMixedProfile() const;

  //@}
};

template <class Generator>
MixedBehaviorProfile<double> GameRep::NewRandomBehaviorProfile(Generator &generator) const
{
  auto profile = MixedBehaviorProfile<double>(Game(const_cast<GameRep *>(this)));
  std::exponential_distribution<> dist(1);
  for (auto player : GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        profile[action] = dist(generator);
      }
    }
  }
  return profile.Normalize();
}

template <class Generator>
MixedBehaviorProfile<Rational> GameRep::NewRandomBehaviorProfile(int p_denom,
                                                                 Generator &generator) const
{
  auto profile = MixedBehaviorProfile<Rational>(Game(const_cast<GameRep *>(this)));
  for (auto player : GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      std::list<Rational> dist = UniformOnSimplex(p_denom, infoset->NumActions(), generator);
      auto prob = dist.cbegin();
      for (auto action : infoset->GetActions()) {
        profile[action] = *prob;
        prob++;
      }
    }
  }
  return profile;
}

} // end namespace Gambit

#endif // LIBGAMBIT_BEHAV_H
