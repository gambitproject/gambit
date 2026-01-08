//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/behav.cc
// Instantiation of behavior profile classes.
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

#include <algorithm>
#include <numeric>

#include "gambit.h"
#include "behavmixed.h"

namespace Gambit {

//========================================================================
//                  MixedBehaviorProfile<T>: Lifecycle
//========================================================================

template <class T>
MixedBehaviorProfile<T>::MixedBehaviorProfile(const Game &p_game)
  : m_probs(p_game->BehavProfileLength()), m_support(BehaviorSupportProfile(p_game)),
    m_gameversion(p_game->GetVersion())
{
  p_game->EnsureInfosetOrdering();
  int index = 1;
  for (const auto &infoset : p_game->GetInfosets()) {
    for (const auto &action : infoset->GetActions()) {
      m_profileIndex[action] = index++;
    }
  }
  SetCentroid();
}

template <class T>
MixedBehaviorProfile<T>::MixedBehaviorProfile(const BehaviorSupportProfile &p_support)
  : m_probs(p_support.BehaviorProfileLength()), m_support(p_support),
    m_gameversion(p_support.GetGame()->GetVersion())
{
  m_support.GetGame()->EnsureInfosetOrdering();
  int index = 1;
  for (const auto &infoset : p_support.GetGame()->GetInfosets()) {
    for (const auto &action : infoset->GetActions()) {
      if (p_support.Contains(action)) {
        m_profileIndex[action] = index++;
      }
      else {
        m_profileIndex[action] = -1;
      }
    }
  }
  SetCentroid();
}

template <class T>
void MixedBehaviorProfile<T>::BehaviorStrat(const GamePlayer &player, const GameNode &p_node,
                                            std::map<GameNode, T> &map_nvals,
                                            std::map<GameNode, T> &map_bvals)
{
  for (const auto &child : p_node->GetChildren()) {
    if (p_node->GetPlayer() == player) {
      if (map_nvals[p_node] > static_cast<T>(0) && map_nvals[child] > static_cast<T>(0)) {
        (*this)[child->GetPriorAction()] = map_nvals[child] / map_nvals[p_node];
      }
    }
    BehaviorStrat(player, child, map_nvals, map_bvals);
  }
}

template <class T>
void MixedBehaviorProfile<T>::RealizationProbs(const MixedStrategyProfile<T> &mp,
                                               const GamePlayer &player,
                                               const std::map<GameInfosetRep *, int> &actions,
                                               GameNodeRep *node, std::map<GameNode, T> &map_nvals,
                                               std::map<GameNode, T> &map_bvals)
{
  T prob;

  for (size_t i = 1; i <= node->m_children.size(); i++) {
    if (node->GetPlayer() && !node->GetPlayer()->IsChance()) {
      if (node->GetPlayer() == player) {
        if (contains(actions, node->m_infoset) &&
            actions.at(node->GetInfoset().get()) == static_cast<int>(i)) {
          prob = static_cast<T>(1);
        }
        else {
          prob = static_cast<T>(0);
        }
      }
      else if (GetSupport().Contains(node->GetInfoset()->GetAction(i))) {
        const int num_actions = GetSupport().GetActions(node->GetInfoset()).size();
        prob = static_cast<T>(1) / static_cast<T>(num_actions);
      }
      else {
        prob = static_cast<T>(0);
      }
    }
    else {
      prob = static_cast<T>(node->m_infoset->GetActionProb(node->m_infoset->GetAction(i)));
    }

    auto child = node->m_children[i - 1];

    map_bvals[child] = prob * map_bvals[node->shared_from_this()];
    map_nvals[child] += map_bvals[child];

    RealizationProbs(mp, player, actions, child.get(), map_nvals, map_bvals);
  }
}

template <class T>
MixedBehaviorProfile<T>::MixedBehaviorProfile(const MixedStrategyProfile<T> &p_profile)
  : m_probs(p_profile.GetGame()->BehavProfileLength()), m_support(p_profile.GetGame()),
    m_gameversion(p_profile.GetGame()->GetVersion())
{
  m_support.GetGame()->EnsureInfosetOrdering();
  int index = 1;
  for (const auto &infoset : p_profile.GetGame()->GetInfosets()) {
    for (const auto &action : infoset->GetActions()) {
      m_profileIndex[action] = index;
      m_probs[index++] = static_cast<T>(0);
    }
  }

  GameNodeRep *root = m_support.GetGame()->GetRoot().get();

  const StrategySupportProfile &support = p_profile.GetSupport();
  const GameRep *game = m_support.GetGame().get();

  for (const auto &player : game->GetPlayers()) {
    std::map<GameNode, T> map_nvals, map_bvals;
    for (const auto &strategy : support.GetStrategies(player)) {
      if (p_profile[strategy] > static_cast<T>(0)) {
        const auto &actions = strategy->m_behav;
        map_bvals[root->shared_from_this()] = p_profile[strategy];
        RealizationProbs(p_profile, player, actions, root, map_nvals, map_bvals);
      }
    }
    map_nvals[root->shared_from_this()] = static_cast<T>(1);
    BehaviorStrat(player, m_support.GetGame()->GetRoot(), map_nvals, map_bvals);
  }
}

template <class T>
MixedBehaviorProfile<T> &
MixedBehaviorProfile<T>::operator=(const MixedBehaviorProfile<T> &p_profile)
{
  if (this == &p_profile) {
    return *this;
  }
  if (m_support != p_profile.m_support) {
    throw MismatchException();
  }
  m_probs = p_profile.m_probs;
  m_gameversion = p_profile.m_gameversion;
  m_cache = p_profile.m_cache;
  return *this;
}

//========================================================================
//              MixedBehaviorProfile<T>: General data access
//========================================================================

template <class T> void MixedBehaviorProfile<T>::SetCentroid()
{
  CheckVersion();
  for (auto infoset : m_support.GetGame()->GetInfosets()) {
    if (!m_support.GetActions(infoset).empty()) {
      T center = T(1) / T(m_support.GetActions(infoset).size());
      for (auto act : m_support.GetActions(infoset)) {
        (*this)[act] = center;
      }
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::UndefinedToCentroid()
{
  CheckVersion();
  const Game efg = m_support.GetGame();
  for (auto infoset : efg->GetInfosets()) {
    if (GetInfosetProb(infoset) > T(0)) {
      continue;
    }
    auto actions = m_support.GetActions(infoset);
    T total =
        std::accumulate(actions.begin(), actions.end(), T(0),
                        [this](T total, GameAction act) { return total + GetActionProb(act); });
    if (total == T(0)) {
      for (auto act : actions) {
        (*this)[act] = T(1) / T(m_support.GetActions(infoset).size());
      }
    }
  }
}

template <class T> MixedBehaviorProfile<T> MixedBehaviorProfile<T>::Normalize() const
{
  CheckVersion();
  auto norm = MixedBehaviorProfile<T>(*this);
  for (auto infoset : m_support.GetGame()->GetInfosets()) {
    if (GetInfosetProb(infoset) == T(0)) {
      continue;
    }
    auto actions = m_support.GetActions(infoset);
    T total =
        std::accumulate(actions.begin(), actions.end(), T(0),
                        [this](T total, GameAction act) { return total + GetActionProb(act); });
    if (total == T(0)) {
      continue;
    }
    for (auto act : actions) {
      norm[act] /= total;
    }
  }
  return norm;
}

template <class T> MixedBehaviorProfile<T> MixedBehaviorProfile<T>::ToFullSupport() const
{
  CheckVersion();
  MixedBehaviorProfile<T> full(GetGame());

  for (auto player : m_support.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        full[action] = (m_support.Contains(action)) ? (*this)[action] : T(0);
      }
    }
  }
  return full;
}

//========================================================================
//              MixedBehaviorProfile<T>: Interesting quantities
//========================================================================

template <class T> T MixedBehaviorProfile<T>::GetLiapValue() const
{
  return MixedStrategyProfile<T>(*this).GetLiapValue();
}

template <class T> T MixedBehaviorProfile<T>::GetAgentLiapValue() const
{
  CheckVersion();
  EnsureRegrets();
  auto value = static_cast<T>(0);
  for (auto infoset : m_support.GetGame()->GetInfosets()) {
    for (auto action : m_support.GetActions(infoset)) {
      value += sqr(std::max(m_cache.m_actionValues[action] - m_cache.m_infosetValues[infoset],
                            static_cast<T>(0)));
    }
  }
  return value;
}

template <class T> const T &MixedBehaviorProfile<T>::GetRealizProb(const GameNode &node) const
{
  CheckVersion();
  EnsureRealizations();
  return m_cache.m_realizProbs[node];
}

template <class T> T MixedBehaviorProfile<T>::GetInfosetProb(const GameInfoset &p_infoset) const
{
  CheckVersion();
  EnsureRealizations();
  return sum_function(p_infoset->GetMembers(),
                      [&](const auto &node) -> T { return m_cache.m_realizProbs[node]; });
}

template <class T> const T &MixedBehaviorProfile<T>::GetBeliefProb(const GameNode &node) const
{
  CheckVersion();
  EnsureBeliefs();
  return m_cache.m_beliefs[node];
}

template <class T> Vector<T> MixedBehaviorProfile<T>::GetPayoff(const GameNode &node) const
{
  CheckVersion();
  EnsureNodeValues();
  Vector<T> ret(node->GetGame()->NumPlayers());
  auto players = node->GetGame()->GetPlayers();
  std::transform(players.begin(), players.end(), ret.begin(),
                 [this, node](GamePlayer player) { return m_cache.m_nodeValues[node][player]; });
  return ret;
}

template <class T>
const T &MixedBehaviorProfile<T>::GetPayoff(const GamePlayer &p_player,
                                            const GameNode &p_node) const
{
  CheckVersion();
  EnsureNodeValues();
  return m_cache.m_nodeValues[p_node][p_player];
}

template <class T> const T &MixedBehaviorProfile<T>::GetPayoff(const GameInfoset &p_infoset) const
{
  CheckVersion();
  EnsureRegrets();
  return m_cache.m_infosetValues[p_infoset];
}

template <class T> T MixedBehaviorProfile<T>::GetActionProb(const GameAction &action) const
{
  CheckVersion();
  if (action->GetInfoset()->GetPlayer()->IsChance()) {
    return static_cast<T>(action->GetInfoset()->GetActionProb(action));
  }
  if (!m_support.Contains(action)) {
    return T(0);
  }
  return m_probs[m_profileIndex.at(action)];
}

template <class T> const T &MixedBehaviorProfile<T>::GetPayoff(const GameAction &act) const
{
  CheckVersion();
  EnsureActionValues();
  return m_cache.m_actionValues[act];
}

template <class T> const T &MixedBehaviorProfile<T>::GetRegret(const GameAction &act) const
{
  CheckVersion();
  EnsureRegrets();
  return m_cache.m_regret.at(act);
}

template <class T> T MixedBehaviorProfile<T>::GetRegret(const GameInfoset &p_infoset) const
{
  CheckVersion();
  EnsureRegrets();
  T br_payoff = maximize_function(p_infoset->GetActions(), [this](const auto &action) -> T {
    return m_cache.m_actionValues.at(action);
  });
  return br_payoff - m_cache.m_infosetValues[p_infoset];
}

template <class T> T MixedBehaviorProfile<T>::GetMaxRegret() const
{
  return MixedStrategyProfile<T>(*this).GetMaxRegret();
}

template <class T> T MixedBehaviorProfile<T>::GetAgentMaxRegret() const
{
  return maximize_function(m_support.GetGame()->GetInfosets(),
                           [this](const auto &infoset) -> T { return this->GetRegret(infoset); });
}

//
// The following routines compute the derivatives of quantities as
// the probability of the action 'p_oppAction' is changed.
// See Turocy (2001), "Computing the Quantal Response Equilibrium
// Correspondence" for details.
// These assume that the profile is interior (totally mixed),
// and that the game is of perfect recall
//

template <class T>
T MixedBehaviorProfile<T>::DiffActionValue(const GameAction &p_action,
                                           const GameAction &p_oppAction) const
{
  CheckVersion();
  EnsureActionValues();
  T deriv = static_cast<T>(0);
  const GameInfoset infoset = p_action->GetInfoset();
  const GamePlayer player = p_action->GetInfoset()->GetPlayer();

  for (auto member : infoset->GetMembers()) {
    const GameNode child = member->GetChild(p_action);

    deriv += DiffRealizProb(member, p_oppAction) *
             (m_cache.m_nodeValues[child][player] - m_cache.m_actionValues[p_action]);
    deriv += m_cache.m_realizProbs[member] *
             DiffNodeValue(member->GetChild(p_action), player, p_oppAction);
  }

  return deriv / GetInfosetProb(p_action->GetInfoset());
}

template <class T>
T MixedBehaviorProfile<T>::DiffRealizProb(const GameNode &p_node,
                                          const GameAction &p_oppAction) const
{
  CheckVersion();
  EnsureActionValues();
  T deriv = static_cast<T>(1);
  bool isPrec = false;
  GameNode node = p_node;
  while (node->GetParent()) {
    if (const GameAction prevAction = node->GetPriorAction(); prevAction != p_oppAction) {
      deriv *= GetActionProb(prevAction);
    }
    else {
      isPrec = true;
    }
    node = node->GetParent();
  }

  return (isPrec) ? deriv : static_cast<T>(0);
}

template <class T>
T MixedBehaviorProfile<T>::DiffNodeValue(const GameNode &p_node, const GamePlayer &p_player,
                                         const GameAction &p_oppAction) const
{
  CheckVersion();
  EnsureNodeValues();

  if (p_node->IsTerminal()) {
    // If we reach a terminal node and haven't encountered p_oppAction,
    // derivative wrt this path is zero.
    return static_cast<T>(0);
  }
  if (p_node->GetInfoset() == p_oppAction->GetInfoset()) {
    // We've encountered the action; since we assume perfect recall,
    // we won't encounter it again, and the downtree value must
    // be the same.
    return m_cache.m_nodeValues[p_node->GetChild(p_oppAction)][p_player];
  }
  return sum_function(p_node->GetActions(), [&](auto action_child) -> T {
    return DiffNodeValue(action_child.second, p_player, p_oppAction) *
           GetActionProb(action_child.first);
  });
}

//========================================================================
//             MixedBehaviorProfile<T>: Cached profile information
//========================================================================

template <class T> void MixedBehaviorProfile<T>::ComputeRealizationProbs() const
{
  m_cache.m_realizProbs.clear();

  const auto &game = m_support.GetGame();
  m_cache.m_realizProbs[game->GetRoot()] = static_cast<T>(1);
  for (const auto &node : game->GetNodes()) {
    const T incomingProb = m_cache.m_realizProbs[node];
    for (auto [action, child] : node->GetActions()) {
      m_cache.m_realizProbs[child] = incomingProb * GetActionProb(action);
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeBeliefs() const
{
  m_cache.m_beliefs.clear();

  for (const auto &infoset : m_support.GetGame()->GetInfosets()) {
    const T infosetProb = sum_function(
        infoset->GetMembers(), [&](const auto &node) -> T { return m_cache.m_realizProbs[node]; });
    if (infosetProb == static_cast<T>(0)) {
      continue;
    }
    for (const auto &node : infoset->GetMembers()) {
      m_cache.m_beliefs[node] = m_cache.m_realizProbs[node] / infosetProb;
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeNodeValues() const
{
  const auto &game = m_support.GetGame();
  m_cache.m_nodeValues.clear();

  for (const auto &node : game->GetNodes(TraversalOrder::Postorder)) {
    auto &vals = m_cache.m_nodeValues[node];
    for (const auto &player : game->GetPlayers()) {
      vals[player] = static_cast<T>(0);
    }
    if (node->GetOutcome()) {
      const GameOutcome &outcome = node->GetOutcome();
      for (const auto &player : game->GetPlayers()) {
        vals[player] += outcome->GetPayoff<T>(player);
      }
    }
    for (auto [action, child] : node->GetActions()) {
      const T p = GetActionProb(action);
      for (const auto &player : game->GetPlayers()) {
        vals[player] += p * m_cache.m_nodeValues[child][player];
      }
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeActionValues() const
{
  const auto &game = m_support.GetGame();
  m_cache.m_actionValues.clear();

  for (const auto &infoset : game->GetInfosets()) {
    const auto &player = infoset->GetPlayer();
    for (const auto &node : infoset->GetMembers()) {
      T belief = m_cache.m_beliefs[node];
      if (belief == static_cast<T>(0)) {
        continue;
      }
      for (auto [action, child] : node->GetActions()) {
        m_cache.m_actionValues[action] += belief * m_cache.m_nodeValues[child][player];
      }
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeActionRegrets() const
{
  for (const auto &infoset : m_support.GetGame()->GetInfosets()) {
    m_cache.m_infosetValues[infoset] =
        sum_function(infoset->GetActions(), [&](const auto &action) -> T {
          return GetActionProb(action) * m_cache.m_actionValues[action];
        });

    auto actions = infoset->GetActions();
    const T brpayoff = maximize_function(infoset->GetActions(), [&](const auto &action) -> T {
      return m_cache.m_actionValues[action];
    });
    for (const auto &action : infoset->GetActions()) {
      m_cache.m_regret[action] =
          std::max(brpayoff - m_cache.m_actionValues[action], static_cast<T>(0));
    }
  }
}

template <class T> bool MixedBehaviorProfile<T>::IsDefinedAt(GameInfoset p_infoset) const
{
  CheckVersion();
  for (auto act : p_infoset->GetActions()) {
    if (GetActionProb(act) > T(0)) {
      return true;
    }
  }
  return false;
}

template <class T> MixedStrategyProfile<T> MixedBehaviorProfile<T>::ToMixedProfile() const
{
  CheckVersion();
  return MixedStrategyProfile<T>(*this);
}

template class MixedBehaviorProfile<double>;
template class MixedBehaviorProfile<Rational>;

} // end namespace Gambit
