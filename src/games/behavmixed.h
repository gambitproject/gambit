//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

#include <random>

#include "game.h"

namespace Gambit {

///
/// MixedBehaviorProfile<T> implements a randomized behavior profile on
/// an extensive game.
///
template <class T> class MixedBehaviorProfile {
protected:
  Vector<T> m_probs;
  BehaviorSupportProfile m_support;
  /// The index into the action profile for a action (-1 if not in support)
  std::map<GameAction, int> m_profileIndex;
  unsigned int m_gameversion;

  struct Cache {
    enum class Level { None, Realizations, Beliefs, NodeValues, ActionValues, Regrets };

    Level m_level{Level::None};
    std::map<GameNode, T> m_realizProbs, m_beliefs;
    std::map<GameNode, std::map<GamePlayer, T>> m_nodeValues;
    std::map<GameInfoset, T> m_infosetValues;
    std::map<GameAction, T> m_actionValues;
    std::map<GameAction, T> m_regret;

    Cache() = default;
    Cache(const Cache &) = default;
    Cache &operator=(const Cache &) = default;
    ~Cache() = default;

    void Clear()
    {
      m_level = Level::None;
      m_realizProbs.clear();
      m_beliefs.clear();
      m_nodeValues.clear();
      m_infosetValues.clear();
      m_actionValues.clear();
      m_regret.clear();
    }
  };

  mutable Cache m_cache;

  /// @name Auxiliary functions for cached computation of interesting values
  //@{
  /// Compute the realisation probabilities of all nodes
  void ComputeRealizationProbs() const;
  /// Compute the realisation probabilities of information sets, and beliefs at
  /// information sets reached with positive probability
  void ComputeBeliefs() const;
  /// Compute the expected payoffs conditional on reaching each node
  void ComputeNodeValues() const;
  /// Compute the expected (conditional) payoffs to each action
  void ComputeActionValues() const;
  /// Compute the conditional value of being at an information set, and corresponding
  /// (agent) action regrets
  void ComputeActionRegrets() const;

  void EnsureRealizations() const
  {
    if (m_cache.m_level >= Cache::Level::Realizations) {
      return;
    }
    ComputeRealizationProbs();
    m_cache.m_level = Cache::Level::Realizations;
  }
  void EnsureBeliefs() const
  {
    EnsureRealizations();
    if (m_cache.m_level >= Cache::Level::Beliefs) {
      return;
    }
    ComputeBeliefs();
    m_cache.m_level = Cache::Level::Beliefs;
  }
  void EnsureNodeValues() const
  {
    EnsureBeliefs();
    if (m_cache.m_level >= Cache::Level::NodeValues) {
      return;
    }
    ComputeNodeValues();
    m_cache.m_level = Cache::Level::NodeValues;
  }
  void EnsureActionValues() const
  {
    EnsureNodeValues();
    if (m_cache.m_level >= Cache::Level::ActionValues) {
      return;
    }
    ComputeActionValues();
    m_cache.m_level = Cache::Level::ActionValues;
  }
  void EnsureRegrets() const
  {
    EnsureActionValues();
    if (m_cache.m_level >= Cache::Level::Regrets) {
      return;
    }
    ComputeActionRegrets();
    m_cache.m_level = Cache::Level::Regrets;
  }

  //@}

  /// @name Converting mixed strategies to behavior
  //@{
  void BehaviorStrat(const GamePlayer &, const GameNode &, std::map<GameNode, T> &,
                     std::map<GameNode, T> &);
  void RealizationProbs(const MixedStrategyProfile<T> &, const GamePlayer &,
                        const std::map<GameInfosetRep *, int> &, GameNodeRep *,
                        std::map<GameNode, T> &, std::map<GameNode, T> &);
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
    m_cache.Clear();
    m_probs = p;
    return *this;
  }
  MixedBehaviorProfile<T> &operator=(const T &x)
  {
    m_cache.Clear();
    m_probs = x;
    return *this;
  }

  //@}

  /// @name Operator overloading
  //@{
  bool operator==(const MixedBehaviorProfile<T> &p_profile) const
  {
    return (m_support == p_profile.m_support && m_probs == p_profile.m_probs);
  }
  bool operator!=(const MixedBehaviorProfile<T> &p_profile) const
  {
    return (m_support != p_profile.m_support || m_probs != p_profile.m_probs);
  }

  const T &operator[](const GameAction &p_action) const
  {
    return m_probs[m_profileIndex.at(p_action)];
  }
  T &operator[](const GameAction &p_action)
  {
    m_cache.Clear();
    return m_probs[m_profileIndex.at(p_action)];
  }

  const T &operator[](int a) const { return m_probs[a]; }
  T &operator[](int a)
  {
    m_cache.Clear();
    return m_probs[a];
  }

  operator const Vector<T> &() const { return m_probs; }
  //@}

  /// @name Initialization, validation
  //@{
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
  size_t BehaviorProfileLength() const { return m_probs.size(); }
  Game GetGame() const { return m_support.GetGame(); }
  const BehaviorSupportProfile &GetSupport() const { return m_support; }
  /// Returns whether the profile has been invalidated by a subsequent revision to the game
  bool IsInvalidated() const { return m_gameversion != m_support.GetGame()->GetVersion(); }

  bool IsDefinedAt(GameInfoset p_infoset) const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetPayoff(const GamePlayer &p_player) const
  {
    CheckVersion();
    EnsureNodeValues();
    return m_cache.m_nodeValues[m_support.GetGame()->GetRoot()][p_player];
  }
  T GetLiapValue() const;
  T GetAgentLiapValue() const;

  const T &GetRealizProb(const GameNode &node) const;
  T GetInfosetProb(const GameInfoset &p_infoset) const;
  const T &GetBeliefProb(const GameNode &node) const;
  Vector<T> GetPayoff(const GameNode &node) const;
  const T &GetPayoff(const GamePlayer &player, const GameNode &node) const;
  const T &GetPayoff(const GameInfoset &p_infoset) const;
  const T &GetPayoff(const GameAction &act) const;
  T GetActionProb(const GameAction &act) const;

  /// @brief Computes the regret to playing \p p_action
  /// @details Computes the regret to the player of playing action \p p_action
  ///          against the profile.  The regret is defined as the difference
  ///          between the best-response payoff and the payoff to playing
  ///          \p p_action.
  /// @param[in] p_action  The action to compute the regret for.
  /// @sa GetRegret(const GameInfoset &) const
  ///     GetAgentMaxRegret() const
  const T &GetRegret(const GameAction &p_action) const;

  /// @brief Computes the regret at information set \p p_infoset
  /// @details Computes the regret at the information set to the player of playing
  ///          their mixed action as specified in the profile.  The regret is defined
  ///          as the difference between the payoff of the best response action and
  ///          the payoff to playing their specified mixed action.
  /// @param[in] p_infoset  The information set to compute the regret at.
  /// @sa GetRegret(const GameAction &) const
  ///     GetAgentMaxRegret() const
  T GetRegret(const GameInfoset &p_infoset) const;

  /// @brief Computes the maximum regret at any information set in the profile
  /// @details Computes the maximum of the regrets of the information sets in the profile.
  /// @sa GetRegret(const GameInfoset &) const
  ///     GetRegret(const GameAction &) const
  ///     GetMaxRegret() const
  T GetAgentMaxRegret() const;

  /// @brief Computes the maximum regret for any player in the profile
  /// @sa GetAgentMaxRegret() const
  ///
  T GetMaxRegret() const;

  T DiffActionValue(const GameAction &action, const GameAction &oppAction) const;
  T DiffRealizProb(const GameNode &node, const GameAction &oppAction) const;
  T DiffNodeValue(const GameNode &node, const GamePlayer &player,
                  const GameAction &oppAction) const;

  MixedStrategyProfile<T> ToMixedProfile() const;

  /// @brief Converts the profile to one on the full support of the game
  MixedBehaviorProfile<T> ToFullSupport() const;
  //@}
};

template <class Generator>
MixedBehaviorProfile<double> GameRep::NewRandomBehaviorProfile(Generator &generator) const
{
  auto profile =
      MixedBehaviorProfile<double>(std::const_pointer_cast<GameRep>(shared_from_this()));
  std::exponential_distribution<> dist(1); // NOLINT(misc-const-correctness)
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
  auto profile =
      MixedBehaviorProfile<Rational>(std::const_pointer_cast<GameRep>(shared_from_this()));
  for (auto player : GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      std::list<Rational> dist =
          UniformOnSimplex(p_denom, infoset->GetActions().size(), generator);
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
