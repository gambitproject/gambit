//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
#include "gametree.h"

namespace Gambit {

//========================================================================
//                  MixedBehaviorProfile<T>: Lifecycle
//========================================================================

template <class T>
MixedBehaviorProfile<T>::MixedBehaviorProfile(const Game &p_game)
  : m_probs(p_game->NumActions()), m_support(BehaviorSupportProfile(p_game)),
    m_gameversion(p_game->GetVersion())
{
  SetCentroid();
}

template <class T>
MixedBehaviorProfile<T>::MixedBehaviorProfile(const BehaviorSupportProfile &p_support)
  : m_probs(p_support.NumActions()), m_support(p_support),
    m_gameversion(p_support.GetGame()->GetVersion())
{
  SetCentroid();
}

template <class T>
void MixedBehaviorProfile<T>::BehaviorStrat(GamePlayer &player, GameNode &p_node,
                                            std::map<GameNode, T> &map_nvals,
                                            std::map<GameNode, T> &map_bvals)
{
  for (auto child : p_node->GetChildren()) {
    if (p_node->GetPlayer() == player) {
      if (map_nvals[p_node] > T(0) && map_nvals[child] > T(0)) {
        (*this)[child->GetPriorAction()] = map_nvals[child] / map_nvals[p_node];
      }
    }
    BehaviorStrat(player, child, map_nvals, map_bvals);
  }
}

template <class T>
void MixedBehaviorProfile<T>::RealizationProbs(const MixedStrategyProfile<T> &mp,
                                               GamePlayer &player, const Array<int> &actions,
                                               GameTreeNodeRep *node,
                                               std::map<GameNode, T> &map_nvals,
                                               std::map<GameNode, T> &map_bvals)
{
  T prob;

  for (int i = 1; i <= node->children.Length(); i++) {
    if (node->GetPlayer() && !node->GetPlayer()->IsChance()) {
      if (node->GetPlayer() == player) {
        if (actions[node->GetInfoset()->GetNumber()] == i) {
          prob = T(1);
        }
        else {
          prob = T(0);
        }
      }
      else if (GetSupport().Contains(node->GetInfoset()->GetAction(i))) {
        int num_actions = GetSupport().NumActions(node->GetInfoset());
        prob = T(1) / T(num_actions);
      }
      else {
        prob = T(0);
      }
    }
    else { // n.GetPlayer() == 0
      prob = T(node->infoset->GetActionProb(i));
    }

    GameTreeNodeRep *child = node->children[i];

    map_bvals[child] = prob * map_bvals[node];
    map_nvals[child] += map_bvals[child];

    RealizationProbs(mp, player, actions, child, map_nvals, map_bvals);
  }
}

template <class T>
MixedBehaviorProfile<T>::MixedBehaviorProfile(const MixedStrategyProfile<T> &p_profile)
  : m_probs(p_profile.GetGame()->NumActions()), m_support(p_profile.GetGame()),
    m_gameversion(p_profile.GetGame()->GetVersion())
{
  static_cast<Vector<T> &>(m_probs) = T(0);

  GameTreeNodeRep *root =
      dynamic_cast<GameTreeNodeRep *>(m_support.GetGame()->GetRoot().operator->());

  const StrategySupportProfile &support = p_profile.GetSupport();
  GameRep *game = m_support.GetGame();

  for (auto player : game->GetPlayers()) {
    std::map<GameNode, T> map_nvals, map_bvals;
    for (auto strategy : support.GetStrategies(player)) {
      if (p_profile[strategy] > T(0)) {
        const Array<int> &actions = strategy->m_behav;
        map_bvals[root] = p_profile[strategy];
        RealizationProbs(p_profile, player, actions, root, map_nvals, map_bvals);
      }
    }
    map_nvals[root] = T(1); // set the root nval
    auto root = m_support.GetGame()->GetRoot();
    BehaviorStrat(player, root, map_nvals, map_bvals);
  }
}

template <class T>
MixedBehaviorProfile<T> &
MixedBehaviorProfile<T>::operator=(const MixedBehaviorProfile<T> &p_profile)
{
  if (this != &p_profile && m_support == p_profile.m_support) {
    InvalidateCache();
    m_probs = p_profile.m_probs;
    m_support = p_profile.m_support;
    m_gameversion = p_profile.m_gameversion;
  }
  return *this;
}

//========================================================================
//               MixedBehaviorProfile<T>: Operator overloading
//========================================================================

template <class T>
bool MixedBehaviorProfile<T>::operator==(const MixedBehaviorProfile<T> &p_profile) const
{
  return (m_support == p_profile.m_support && (DVector<T> &)*this == (DVector<T> &)p_profile);
}

//========================================================================
//              MixedBehaviorProfile<T>: General data access
//========================================================================

template <class T> void MixedBehaviorProfile<T>::SetCentroid()
{
  CheckVersion();
  for (auto infoset : m_support.GetGame()->GetInfosets()) {
    if (infoset->NumActions() > 0) {
      T center = (T(1) / T(infoset->NumActions()));
      for (auto act : infoset->GetActions()) {
        (*this)[act] = center;
      }
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::UndefinedToCentroid()
{
  CheckVersion();
  Game efg = m_support.GetGame();
  for (auto infoset : efg->GetInfosets()) {
    if (GetInfosetProb(infoset) > T(0)) {
      continue;
    }
    auto actions = infoset->GetActions();
    T total =
        std::accumulate(actions.begin(), actions.end(), T(0),
                        [this](T total, GameAction act) { return total + GetActionProb(act); });
    if (total == T(0)) {
      for (auto act : infoset->GetActions()) {
        (*this)[act] = T(1) / T(infoset->NumActions());
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
    auto actions = infoset->GetActions();
    T total =
        std::accumulate(actions.begin(), actions.end(), T(0),
                        [this](T total, GameAction act) { return total + GetActionProb(act); });
    if (total == T(0)) {
      continue;
    }
    for (auto act : infoset->GetActions()) {
      norm[act] /= total;
    }
  }
  return norm;
}

//========================================================================
//              MixedBehaviorProfile<T>: Interesting quantities
//========================================================================

template <class T> T MixedBehaviorProfile<T>::GetLiapValue() const
{
  CheckVersion();
  ComputeSolutionData();

  auto value = T(0);
  for (auto player : m_support.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        value += sqr(std::max(GetPayoff(action) - GetPayoff(infoset), T(0)));
      }
    }
  }
  return value;
}

template <class T> const T &MixedBehaviorProfile<T>::GetRealizProb(const GameNode &node) const
{
  CheckVersion();
  ComputeSolutionData();
  return map_realizProbs[node];
}

template <class T> T MixedBehaviorProfile<T>::GetInfosetProb(const GameInfoset &iset) const
{
  CheckVersion();
  ComputeSolutionData();
  T prob = T(0);
  for (auto iset : iset->GetMembers()) {
    prob += map_realizProbs[iset];
  }
  return prob;
}

template <class T> const T &MixedBehaviorProfile<T>::GetBeliefProb(const GameNode &node) const
{
  CheckVersion();
  ComputeSolutionData();
  return map_beliefs[node];
}

template <class T> Vector<T> MixedBehaviorProfile<T>::GetPayoff(const GameNode &node) const
{
  CheckVersion();
  ComputeSolutionData();
  Vector<T> ret(node->GetGame()->NumPlayers());
  auto players = node->GetGame()->GetPlayers();
  std::transform(players.begin(), players.end(), ret.begin(),
                 [this, node](GamePlayer player) { return map_nodeValues[node][player]; });
  return ret;
}

template <class T>
const T &MixedBehaviorProfile<T>::GetPayoff(const GamePlayer &p_player,
                                            const GameNode &p_node) const
{
  CheckVersion();
  ComputeSolutionData();
  return map_nodeValues[p_node][p_player];
}

template <class T> const T &MixedBehaviorProfile<T>::GetPayoff(const GameInfoset &iset) const
{
  CheckVersion();
  ComputeSolutionData();
  return map_infosetValues[iset];
}

template <class T> T MixedBehaviorProfile<T>::GetActionProb(const GameAction &action) const
{
  CheckVersion();
  if (action->GetInfoset()->GetPlayer()->IsChance()) {
    GameTreeInfosetRep *infoset =
        dynamic_cast<GameTreeInfosetRep *>(action->GetInfoset().operator->());
    return static_cast<T>(infoset->GetActionProb(action->GetNumber()));
  }
  else if (!m_support.Contains(action)) {
    return T(0);
  }
  else {
    return m_probs(action->GetInfoset()->GetPlayer()->GetNumber(),
                   action->GetInfoset()->GetNumber(), m_support.GetIndex(action));
  }
}

template <class T> const T &MixedBehaviorProfile<T>::GetPayoff(const GameAction &act) const
{
  CheckVersion();
  ComputeSolutionData();
  return map_actionValues[act];
}

template <class T> const T &MixedBehaviorProfile<T>::GetRegret(const GameAction &act) const
{
  CheckVersion();
  ComputeSolutionData();
  return map_regret[act];
}

template <class T> T MixedBehaviorProfile<T>::GetRegret(const GameInfoset &p_infoset) const
{
  auto actions = p_infoset->GetActions();
  T br_payoff = std::accumulate(
      std::next(actions.begin()), actions.end(), GetPayoff(*actions.begin()),
      [this](const T &x, const GameAction &action) { return std::max(x, GetPayoff(action)); });
  return br_payoff - GetPayoff(p_infoset);
}

template <class T> T MixedBehaviorProfile<T>::GetMaxRegret() const
{
  auto infosets = m_support.GetGame()->GetInfosets();
  return std::accumulate(
      infosets.begin(), infosets.end(), T(0),
      [this](const T &x, const GameInfoset &infoset) { return std::max(x, GetRegret(infoset)); });
}

template <class T>
void MixedBehaviorProfile<T>::GetPayoff(const GameNode &node, const T &prob,
                                        const GamePlayer &player, T &value) const
{
  if (node->GetOutcome()) {
    value += prob * static_cast<T>(node->GetOutcome()->GetPayoff(player));
  }

  if (!node->IsTerminal()) {
    if (node->GetPlayer()->IsChance()) {
      // chance player
      for (auto child : node->GetChildren()) {
        GetPayoff(child, prob * static_cast<T>(GetActionProb(child->GetPriorAction())), player,
                  value);
      }
    }
    else {
      for (auto child : node->GetChildren()) {
        GetPayoff(child, prob * GetActionProb(child->GetPriorAction()), player, value);
      }
    }
  }
}

template <class T> T MixedBehaviorProfile<T>::GetPayoff(int pl) const
{
  CheckVersion();
  T value = T(0);
  auto rootNode = m_support.GetGame()->GetRoot();
  auto player = m_support.GetGame()->GetPlayer(pl);
  GetPayoff(rootNode, T(1), player, value);
  return value;
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
  ComputeSolutionData();
  T deriv = T(0);
  GameInfoset infoset = p_action->GetInfoset();
  GamePlayer player = p_action->GetInfoset()->GetPlayer();

  for (auto member : infoset->GetMembers()) {
    GameNode child = member->GetChild(p_action);

    deriv += DiffRealizProb(member, p_oppAction) *
             (map_nodeValues[child][player] - map_actionValues[p_action]);
    deriv += map_realizProbs[member] *
             DiffNodeValue(member->GetChild(p_action->GetNumber()), player, p_oppAction);
  }

  return deriv / GetInfosetProb(p_action->GetInfoset());
}

template <class T>
T MixedBehaviorProfile<T>::DiffRealizProb(const GameNode &p_node,
                                          const GameAction &p_oppAction) const
{
  CheckVersion();
  ComputeSolutionData();
  T deriv = T(1);
  bool isPrec = false;
  GameNode node = p_node;
  while (node->GetParent()) {
    GameAction prevAction = node->GetPriorAction();
    if (prevAction != p_oppAction) {
      deriv *= GetActionProb(prevAction);
    }
    else {
      isPrec = true;
    }
    node = node->GetParent();
  }

  return (isPrec) ? deriv : T(0);
}

template <class T>
T MixedBehaviorProfile<T>::DiffNodeValue(const GameNode &p_node, const GamePlayer &p_player,
                                         const GameAction &p_oppAction) const
{
  CheckVersion();
  ComputeSolutionData();

  if (p_node->NumChildren() > 0) {
    GameInfoset infoset = p_node->GetInfoset();

    if (infoset == p_oppAction->GetInfoset()) {
      // We've encountered the action; since we assume perfect recall,
      // we won't encounter it again, and the downtree value must
      // be the same.
      return map_nodeValues[p_node->GetChild(p_oppAction)][p_player];
    }
    else {
      T deriv = T(0);
      for (auto action : infoset->GetActions()) {
        deriv += (DiffNodeValue(p_node->GetChild(action), p_player, p_oppAction) *
                  GetActionProb(action));
      }
      return deriv;
    }
  }
  else {
    // If we reach a terminal node and haven't encountered p_oppAction,
    // derivative wrt this path is zero.
    return T(0);
  }
}

//========================================================================
//             MixedBehaviorProfile<T>: Cached profile information
//========================================================================

// compute realization probabilities for nodes and isets.
template <class T>
void MixedBehaviorProfile<T>::ComputePass1_realizProbs(const GameNode &node) const
{
  map_realizProbs[node] = (node->GetParent()) ? map_realizProbs[node->GetParent()] *
                                                    GetActionProb(node->GetPriorAction())
                                              : T(1);

  for (auto childNode : node->GetChildren()) {
    ComputePass1_realizProbs(childNode);
  }
}

template <class T>
void MixedBehaviorProfile<T>::ComputePass2_beliefs_nodeValues_actionValues(
    const GameNode &node) const
{
  if (node->GetOutcome()) {
    GameOutcome outcome = node->GetOutcome();
    for (auto player : m_support.GetGame()->GetPlayers()) {
      map_nodeValues[node][player] += static_cast<T>(outcome->GetPayoff(player));
    }
  }

  if (node->IsTerminal()) {
    return;
  }

  GameInfoset iset = node->GetInfoset();
  auto nodes = iset->GetMembers();
  T infosetProb =
      std::accumulate(nodes.begin(), nodes.end(), T(0),
                      [this](T total, GameNode node) { return total + map_realizProbs[node]; });

  if (infosetProb != T(0)) {
    map_beliefs[node] = map_realizProbs[node] / infosetProb;
  }

  // push down payoffs from outcomes attached to non-terminal nodes
  for (auto child : node->GetChildren()) {
    map_nodeValues[child] = map_nodeValues[node];
  }

  for (auto player : m_support.GetGame()->GetPlayers()) {
    map_nodeValues[node][player] = T(0);
  }

  for (auto child : node->GetChildren()) {
    ComputePass2_beliefs_nodeValues_actionValues(child);

    GameAction act = child->GetPriorAction();

    for (auto player : m_support.GetGame()->GetPlayers()) {
      map_nodeValues[node][player] += GetActionProb(act) * map_nodeValues[child][player];
    }

    if (!iset->IsChanceInfoset()) {
      map_actionValues[act] += (infosetProb != T(0))
                                   ? map_beliefs[node] * map_nodeValues[child][iset->GetPlayer()]
                                   : T(0);
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputePass3_infosetValues_regret() const
{
  // Populate
  for (auto infoset : m_support.GetGame()->GetInfosets()) {
    map_infosetValues[infoset] = T(0);
    for (auto action : infoset->GetActions()) {
      map_infosetValues[infoset] += GetActionProb(action) * map_actionValues[action];
    }
    auto actions = infoset->GetActions();
    T brpayoff = map_actionValues[actions.front()];
    for (auto action : infoset->GetActions()) {
      brpayoff = std::max(brpayoff, map_actionValues[action]);
    }
    for (auto action : infoset->GetActions()) {
      map_regret[action] = brpayoff - map_actionValues[action];
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeSolutionData() const
{
  auto rootNode = m_support.GetGame()->GetRoot();
  if (contains(map_realizProbs, rootNode)) {
    // cache is valid, don't compute anything, simply return
    return;
  }
  ComputePass1_realizProbs(rootNode);
  ComputePass2_beliefs_nodeValues_actionValues(rootNode);
  ComputePass3_infosetValues_regret();
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
