//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/behavmixed.cc
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

#include "games.h"
#include "gametree.h"
#include "behavmixed.h"

namespace Gambit {

namespace {

/// The data needed to evaluate one player's deviations against a fixed behavior profile
template <class T> struct DeviationProblem {
  const TreeLayout &layout;
  const std::vector<T> &actionProb; ///< by node: probability of the action leading to it
  size_t player;                    ///< position of the deviating player
};

template <class T> const T &Payoff(const DeviationProblem<T> &p_problem, size_t p_node)
{
  const auto &layout = p_problem.layout;
  return layout.template GetNumbers<T>().payoffs[p_node * layout.numPlayers + p_problem.player];
}

template <class T> bool IsOwnNode(const DeviationProblem<T> &p_problem, size_t p_node)
{
  const auto &layout = p_problem.layout;
  const int info = layout.infoset[p_node];
  return info >= 0 && std::cmp_less(info, layout.numPersonalInfosets) &&
         std::cmp_equal(layout.infosets[info].player, p_problem.player);
}

/// The child of a node reached by the action in position p_slot at the node's information set
size_t ChildAt(const TreeLayout &p_layout, size_t p_node, size_t p_slot)
{
  return p_layout.childList[p_layout.firstChild[p_node] + p_slot];
}

/// Probability of reaching each node when the deviating player chooses actions to reach it
template <class T> std::vector<T> OthersReachProbs(const DeviationProblem<T> &p_problem)
{
  const auto &layout = p_problem.layout;
  std::vector<T> reach(layout.parent.size(), static_cast<T>(1));
  for (size_t k = 1; k < reach.size(); k++) {
    const size_t parent = layout.parent[k];
    reach[k] =
        IsOwnNode(p_problem, parent) ? reach[parent] : reach[parent] * p_problem.actionProb[k];
  }
  return reach;
}

/// Memoized state of the backward induction on a player's information sets
template <class T> struct BackwardInduction {
  std::vector<T> reach;                    ///< by node, probability of reaching it absent own play
  std::vector<std::optional<T>> value;     ///< by node, once evaluated
  std::vector<std::optional<size_t>> best; ///< by information set, the best action's position
};

/// The position of the action maximizing the player's value at an information set, given
/// that the values below its members' children have been evaluated
template <class T>
size_t BestActionPosition(const DeviationProblem<T> &p_problem,
                          const BackwardInduction<T> &p_state, size_t p_infoset)
{
  const auto &layout = p_problem.layout;
  size_t best = 0;
  T bestValue = static_cast<T>(0);
  for (size_t position = 0; position < layout.infosets[p_infoset].numActions; position++) {
    T actionValue = static_cast<T>(0);
    for (const size_t member : layout.infosets[p_infoset].members) {
      actionValue += p_state.reach[member] * *p_state.value[ChildAt(layout, member, position)];
    }
    if (position == 0 || actionValue > bestValue) {
      best = position;
      bestValue = actionValue;
    }
  }
  return best;
}

/// Evaluate the value of the root, with the player best-responding at each information set.
/// A node is evaluated once the nodes it depends on are; the dependencies of a node of the
/// player are, by perfect recall, never its own ancestors, so the walk terminates.
template <class T>
void EvaluateBackwardInduction(const DeviationProblem<T> &p_problem, BackwardInduction<T> &p_state)
{
  const auto &layout = p_problem.layout;
  std::vector<size_t> stack{0};
  const auto pushMissing = [&](size_t p_child) {
    if (!p_state.value[p_child]) {
      stack.push_back(p_child);
    }
  };
  while (!stack.empty()) {
    const size_t node = stack.back();
    if (p_state.value[node]) {
      stack.pop_back();
      continue;
    }
    const size_t size = stack.size();
    T value = Payoff(p_problem, node);
    if (!IsOwnNode(p_problem, node)) {
      for (size_t j = 0; j < layout.numChildren[node]; j++) {
        pushMissing(ChildAt(layout, node, j));
      }
      if (stack.size() > size) {
        continue;
      }
      for (size_t j = 0; j < layout.numChildren[node]; j++) {
        const size_t child = ChildAt(layout, node, j);
        value += p_problem.actionProb[child] * *p_state.value[child];
      }
    }
    else {
      const auto info = static_cast<size_t>(layout.infoset[node]);
      if (!p_state.best[info]) {
        for (const size_t member : layout.infosets[info].members) {
          for (size_t j = 0; j < layout.numChildren[member]; j++) {
            pushMissing(ChildAt(layout, member, j));
          }
        }
        if (stack.size() > size) {
          continue;
        }
        p_state.best[info] = BestActionPosition(p_problem, p_state, info);
      }
      const size_t child = ChildAt(layout, node, *p_state.best[info]);
      pushMissing(child);
      if (stack.size() > size) {
        continue;
      }
      value += *p_state.value[child];
    }
    p_state.value[node] = value;
    stack.pop_back();
  }
}

/// Best-response value of a player with perfect recall, by backward induction on the
/// player's information sets
template <class T> T PerfectRecallBestResponse(const DeviationProblem<T> &p_problem)
{
  BackwardInduction<T> state{OthersReachProbs(p_problem),
                             std::vector<std::optional<T>>(p_problem.layout.parent.size()),
                             std::vector<std::optional<size_t>>(p_problem.layout.infosets.size())};
  EvaluateBackwardInduction(p_problem, state);
  return *state.value[0];
}

/// A node awaiting evaluation in the search over pure behavior profiles
template <class T> struct PendingNode {
  size_t node;
  T weight; ///< probability of reaching the node, given the player's choices so far
};

/// A partially completed walk of the tree under the player's choices so far
template <class T> struct SearchFrame {
  std::vector<PendingNode<T>> pending;
  T value; ///< weighted payoff of the nodes visited so far
  /// Set while the frame is branching on the information set of an unvisited node of the player
  std::optional<size_t> branchInfoset;
  size_t branchNode{0};
  T branchWeight{};
  size_t nextPosition{0}; ///< the next action position to try at the branching information set
};

/// Best-response value of a player without absent-mindedness, by depth-first enumeration of
/// the player's pure behavior profiles, in which an information set is branched on only when
/// the walk of the tree reaches it
template <class T> T PureEnumerationBestResponse(const DeviationProblem<T> &p_problem)
{
  const auto &layout = p_problem.layout;
  std::vector<std::optional<size_t>> choice(layout.infosets.size());
  std::optional<T> best;
  std::vector<SearchFrame<T>> frames;
  frames.push_back({{{0, static_cast<T>(1)}}, static_cast<T>(0), std::nullopt, 0, T{}, 0});
  while (!frames.empty()) {
    auto &frame = frames.back();
    if (!frame.branchInfoset) {
      while (!frame.pending.empty()) {
        const auto [node, weight] = frame.pending.back();
        frame.pending.pop_back();
        if (weight == static_cast<T>(0)) {
          continue;
        }
        frame.value += weight * Payoff(p_problem, node);
        if (!IsOwnNode(p_problem, node)) {
          for (size_t j = 0; j < layout.numChildren[node]; j++) {
            const size_t child = ChildAt(layout, node, j);
            frame.pending.push_back({child, weight * p_problem.actionProb[child]});
          }
          continue;
        }
        const auto info = static_cast<size_t>(layout.infoset[node]);
        if (choice[info]) {
          frame.pending.push_back({ChildAt(layout, node, *choice[info]), weight});
          continue;
        }
        frame.branchInfoset = info;
        frame.branchNode = node;
        frame.branchWeight = weight;
        break;
      }
      if (!frame.branchInfoset) {
        if (!best || frame.value > *best) {
          best = frame.value;
        }
        frames.pop_back();
        continue;
      }
    }
    const size_t info = *frame.branchInfoset;
    if (frame.nextPosition == layout.infosets[info].numActions) {
      choice[info] = std::nullopt;
      frames.pop_back();
      continue;
    }
    const size_t position = frame.nextPosition++;
    choice[info] = position;
    SearchFrame<T> branch{frame.pending, frame.value, std::nullopt, 0, T{}, 0};
    branch.pending.push_back({ChildAt(layout, frame.branchNode, position), frame.branchWeight});
    frames.push_back(std::move(branch));
  }
  return *best;
}

} // end anonymous namespace

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
  InitializeLayout();
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
  InitializeLayout();
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
        if (actions.contains(node->m_infoset) &&
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
  InitializeLayout();

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
MixedBehaviorProfile<T>::MixedBehaviorProfile(const MixedSequenceProfile<T> &p_profile)
  : m_probs(p_profile.GetGame()->BehavProfileLength()),
    m_support(BehaviorSupportProfile(p_profile.GetGame())),
    m_gameversion(p_profile.GetGame()->GetVersion())
{
  const Game game = p_profile.GetGame();
  game->EnsureInfosetOrdering();
  int index = 1;
  for (const auto &infoset : game->GetInfosets()) {
    for (const auto &action : infoset->GetActions()) {
      m_profileIndex[action] = index++;
    }
  }
  InitializeLayout();
  for (auto player : game->GetPlayers()) {
    for (auto sequence : player->GetSequences()) {
      if (!sequence->GetAction()) {
        continue;
      }
      const T parentProb = p_profile[sequence->GetParent()];
      (*this)[sequence->GetAction()] =
          (parentProb > T{0}) ? p_profile[sequence] / parentProb : T{0};
    }
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
  m_profileIndex = p_profile.m_profileIndex;
  m_gameversion = p_profile.m_gameversion;
  // The cache is aligned with the layout of the version of the game it was computed for
  m_layout = p_profile.m_layout;
  m_layoutProfileIndex = p_profile.m_layoutProfileIndex;
  m_cache = p_profile.m_cache;
  return *this;
}

template <class T> void MixedBehaviorProfile<T>::InitializeLayout()
{
  const Game game = m_support.GetGame();
  m_layout = game->GetTreeLayout();
  // The layout numbers the actions at personal information sets in the order of
  // GameRep::GetInfosets(), as does m_profileIndex on the full game
  m_layoutProfileIndex.assign(m_layout->numActions + 1, -1);
  size_t layoutIndex = 1;
  for (const auto &infoset : game->GetInfosets()) {
    for (const auto &action : infoset->GetActions()) {
      m_layoutProfileIndex[layoutIndex++] = m_profileIndex.at(action);
    }
  }
}

template <class T>
std::optional<size_t> MixedBehaviorProfile<T>::NodeIndex(const GameNode &p_node) const
{
  if (!p_node || p_node->m_game != m_support.GetGame().get()) {
    return std::nullopt;
  }
  // Node numbers follow the order of GameRep::GetNodes(), as does the layout
  return p_node->GetNumber() - 1;
}

template <class T>
std::optional<size_t> MixedBehaviorProfile<T>::InfosetIndex(const GameInfoset &p_infoset) const
{
  const auto entry = m_layout->infosetIndex.find(p_infoset.get());
  if (entry == m_layout->infosetIndex.end()) {
    return std::nullopt;
  }
  return entry->second;
}

template <class T>
std::optional<size_t> MixedBehaviorProfile<T>::PlayerIndex(const GamePlayer &p_player) const
{
  if (!p_player || p_player->IsChance() || p_player->GetGame() != m_support.GetGame()) {
    return std::nullopt;
  }
  return p_player->GetNumber() - 1;
}

template <class T> T MixedBehaviorProfile<T>::LayoutActionProb(size_t p_node) const
{
  const size_t action = m_layout->action[p_node];
  if (action == 0) {
    return m_layout->template GetNumbers<T>().chanceProb[p_node];
  }
  return LayoutProfileProb(action);
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
  MixedBehaviorProfile full(GetGame());

  for (auto player : m_support.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        full[action] = (m_support.Contains(action)) ? (*this)[action] : T{0};
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
  m_support.GetGame()->EnsureStrategies();
  return MixedStrategyProfile<T>(*this).GetLiapValue();
}

template <class T> T MixedBehaviorProfile<T>::GetAgentLiapValue() const
{
  CheckVersion();
  EnsureRegrets();
  T value{0};
  for (auto infoset : m_support.GetGame()->GetInfosets()) {
    if (GetInfosetProb(infoset) == T{0}) {
      continue;
    }
    const auto &entry = m_layout->infosets[*InfosetIndex(infoset)];
    for (auto action : m_support.GetActions(infoset)) {
      value += sqr(std::max(m_cache.m_actionValues[entry.firstAction + action->GetNumber() - 1] -
                                m_cache.m_infosetValues[*InfosetIndex(infoset)],
                            static_cast<T>(0)));
    }
  }
  return value;
}

template <class T> const T &MixedBehaviorProfile<T>::GetRealizProb(const GameNode &node) const
{
  CheckVersion();
  EnsureRealizations();
  static const T zero(0);
  const auto index = NodeIndex(node);
  return (index) ? m_cache.m_realizProbs[*index] : zero;
}

template <class T> T MixedBehaviorProfile<T>::GetInfosetProb(const GameInfoset &p_infoset) const
{
  CheckVersion();
  EnsureRealizations();
  const auto index = InfosetIndex(p_infoset);
  return (index) ? m_cache.m_infosetProbs[*index] : T(0);
}

template <class T>
std::optional<T> MixedBehaviorProfile<T>::GetBeliefProb(const GameNode &node) const
{
  CheckVersion();
  EnsureBeliefs();
  if (!node->GetInfoset() || GetInfosetProb(node->GetInfoset()) == T{0}) {
    return std::nullopt;
  }
  const auto index = NodeIndex(node);
  return (index) ? m_cache.m_beliefs[*index] : T(0);
}

template <class T> Vector<T> MixedBehaviorProfile<T>::GetPayoff(const GameNode &node) const
{
  CheckVersion();
  EnsureNodeValues();
  Vector<T> ret(node->GetGame()->NumPlayers());
  const auto index = NodeIndex(node);
  for (size_t pl = 1; pl <= ret.size(); pl++) {
    ret[pl] = (index) ? m_cache.m_nodeValues[*index * m_layout->numPlayers + pl - 1] : T(0);
  }
  return ret;
}

template <class T>
const T &MixedBehaviorProfile<T>::GetPayoff(const GamePlayer &p_player,
                                            const GameNode &p_node) const
{
  CheckVersion();
  EnsureNodeValues();
  const auto node = NodeIndex(p_node);
  const auto player = PlayerIndex(p_player);
  if (!node || !player) {
    throw std::out_of_range("Node or player is not part of the game of the profile");
  }
  return m_cache.m_nodeValues[*node * m_layout->numPlayers + *player];
}

template <class T>
std::optional<T> MixedBehaviorProfile<T>::GetPayoff(const GameInfoset &p_infoset) const
{
  CheckVersion();
  EnsureRegrets();
  if (GetInfosetProb(p_infoset) == T{0}) {
    return std::nullopt;
  }
  return m_cache.m_infosetValues[*InfosetIndex(p_infoset)];
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

template <class T> std::optional<T> MixedBehaviorProfile<T>::GetPayoff(const GameAction &act) const
{
  CheckVersion();
  EnsureActionValues();
  if (GetInfosetProb(act->GetInfoset()) == T{0}) {
    return std::nullopt;
  }
  const size_t info = *InfosetIndex(act->GetInfoset());
  if (info >= m_layout->numPersonalInfosets) {
    return T(0);
  }
  return m_cache.m_actionValues[m_layout->infosets[info].firstAction + act->GetNumber() - 1];
}

template <class T> T MixedBehaviorProfile<T>::GetRegret(const GameAction &act) const
{
  CheckVersion();
  EnsureRegrets();
  if (GetInfosetProb(act->GetInfoset()) == T{0}) {
    return T{0};
  }
  const size_t info = *InfosetIndex(act->GetInfoset());
  if (info >= m_layout->numPersonalInfosets) {
    throw std::out_of_range("Regrets are not defined for chance actions");
  }
  return m_cache.m_regret[m_layout->infosets[info].firstAction + act->GetNumber() - 1];
}

template <class T> T MixedBehaviorProfile<T>::GetRegret(const GameInfoset &p_infoset) const
{
  CheckVersion();
  EnsureRegrets();
  if (GetInfosetProb(p_infoset) == T{0}) {
    return T{0};
  }
  const size_t info = *InfosetIndex(p_infoset);
  const auto &entry = m_layout->infosets[info];
  T br_payoff = m_cache.m_actionValues[entry.firstAction];
  for (size_t a = entry.firstAction + 1; a < entry.firstAction + entry.numActions; a++) {
    br_payoff = std::max(br_payoff, m_cache.m_actionValues[a]);
  }
  return br_payoff - m_cache.m_infosetValues[info];
}

template <class T> T MixedBehaviorProfile<T>::GetMaxRegret() const
{
  CheckVersion();
  const auto game = m_support.GetGame();
  if (!m_layout->reentries.empty()) {
    throw UndefinedException(
        "Player regret is not defined for a behavior profile of a game with absent-mindedness");
  }
  const bool perfectRecall = game->IsPerfectRecall();
  std::vector<T> actionProb(m_layout->parent.size(), static_cast<T>(1));
  for (size_t k = 1; k < actionProb.size(); k++) {
    actionProb[k] = LayoutActionProb(k);
  }
  T maxRegret = static_cast<T>(0);
  for (const auto &player : game->GetPlayers()) {
    const DeviationProblem<T> problem{*m_layout, actionProb, *PlayerIndex(player)};
    const T bestResponse =
        perfectRecall ? PerfectRecallBestResponse(problem) : PureEnumerationBestResponse(problem);
    maxRegret = std::max(maxRegret, bestResponse - GetPayoff(player));
  }
  return maxRegret;
}

template <class T> T MixedBehaviorProfile<T>::GetAgentMaxRegret() const
{
  auto infosets = m_support.GetGame()->GetInfosets();
  if (infosets.size() == 0) {
    return T{0};
  }
  return maximize_function(infosets,
                           [this](const auto &infoset) -> T { return this->GetRegret(infoset); });
}

//========================================================================
//             MixedBehaviorProfile<T>: Cached profile information
//========================================================================

template <class T> void MixedBehaviorProfile<T>::ComputeRealizationProbs() const
{
  const auto &layout = *m_layout;
  const size_t numNodes = layout.parent.size();
  auto &realiz = m_cache.m_realizProbs;
  realiz.assign(numNodes, static_cast<T>(0));
  realiz[0] = static_cast<T>(1);
  for (size_t k = 1; k < numNodes; k++) {
    realiz[k] = realiz[layout.parent[k]] * LayoutActionProb(k);
  }

  auto &infosetProbs = m_cache.m_infosetProbs;
  infosetProbs.assign(layout.infosets.size(), static_cast<T>(0));
  for (size_t i = 0; i < layout.infosets.size(); i++) {
    for (const size_t member : layout.infosets[i].members) {
      infosetProbs[i] += realiz[member];
    }
  }
  for (const auto &[infoset, node] : layout.reentries) {
    infosetProbs[infoset] -= realiz[node];
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeBeliefs() const
{
  // Normalise each member's realization probability by the infoset's upper-frontier probability
  // (m_infosetProbs, computed in ComputeRealizationProbs), following Halpern and Pass (2021).
  // For an absent-minded infoset the frontier excludes the reentry members, so the member beliefs
  // may sum to above 1; for a non-absent-minded infoset the frontier is all members and this is
  // the standard Selten (1975) normalization.
  const auto &layout = *m_layout;
  m_cache.m_beliefs.assign(layout.parent.size(), static_cast<T>(0));
  for (size_t i = 0; i < layout.numPersonalInfosets; i++) {
    const T infosetProb = m_cache.m_infosetProbs[i];
    if (infosetProb == static_cast<T>(0)) {
      continue;
    }
    for (const size_t member : layout.infosets[i].members) {
      m_cache.m_beliefs[member] = m_cache.m_realizProbs[member] / infosetProb;
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeNodeValues() const
{
  const auto &layout = *m_layout;
  const auto &payoffs = layout.template GetNumbers<T>().payoffs;
  const size_t numPlayers = layout.numPlayers;
  auto &values = m_cache.m_nodeValues;
  values.resize(layout.parent.size() * numPlayers);

  // Children follow their parents in the layout, so a reverse sweep is a postorder traversal
  for (size_t k = layout.parent.size(); k-- > 0;) {
    const size_t node = k * numPlayers;
    for (size_t pl = 0; pl < numPlayers; pl++) {
      values[node + pl] = static_cast<T>(0);
    }
    for (size_t pl = 0; pl < numPlayers; pl++) {
      values[node + pl] += payoffs[node + pl];
    }
    for (size_t j = 0; j < layout.numChildren[k]; j++) {
      const size_t child = layout.childList[layout.firstChild[k] + j];
      const T p = LayoutActionProb(child);
      for (size_t pl = 0; pl < numPlayers; pl++) {
        values[node + pl] += p * values[child * numPlayers + pl];
      }
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeActionValues() const
{
  const auto &layout = *m_layout;
  const size_t numPlayers = layout.numPlayers;
  m_cache.m_actionValues.assign(layout.numActions + 1, static_cast<T>(0));

  for (size_t i = 0; i < layout.numPersonalInfosets; i++) {
    const auto &infoset = layout.infosets[i];
    for (const size_t member : infoset.members) {
      const T belief = m_cache.m_beliefs[member];
      if (belief == static_cast<T>(0)) {
        continue;
      }
      for (size_t j = 0; j < layout.numChildren[member]; j++) {
        const size_t child = layout.childList[layout.firstChild[member] + j];
        m_cache.m_actionValues[layout.action[child]] +=
            belief * m_cache.m_nodeValues[child * numPlayers + infoset.player];
      }
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::ComputeActionRegrets() const
{
  const auto &layout = *m_layout;
  m_cache.m_infosetValues.assign(layout.infosets.size(), static_cast<T>(0));
  m_cache.m_regret.assign(layout.numActions + 1, static_cast<T>(0));
  const auto &values = m_cache.m_actionValues;

  for (size_t i = 0; i < layout.numPersonalInfosets; i++) {
    const auto &infoset = layout.infosets[i];
    const size_t first = infoset.firstAction;
    const size_t last = first + infoset.numActions;
    T infosetValue = static_cast<T>(0);
    for (size_t a = first; a < last; a++) {
      infosetValue += LayoutProfileProb(a) * values[a];
    }
    m_cache.m_infosetValues[i] = infosetValue;
    T brpayoff = values[first];
    for (size_t a = first + 1; a < last; a++) {
      brpayoff = std::max(brpayoff, values[a]);
    }
    for (size_t a = first; a < last; a++) {
      m_cache.m_regret[a] = std::max(brpayoff - values[a], static_cast<T>(0));
    }
  }
}

template <class T> void MixedBehaviorProfile<T>::DiffActionValues(Matrix<T> &p_derivs) const
{
  CheckVersion();
  EnsureActionValues();
  const auto &layout = *m_layout;
  Matrix<T> derivs(layout.numActions, layout.numActions);
  Gambit::DiffActionValues(
      layout, m_cache.m_beliefs, m_cache.m_nodeValues, m_cache.m_actionValues,
      [this](size_t p_from, size_t p_to) {
        T prob = static_cast<T>(1);
        for (size_t k = p_to; k != p_from; k = m_layout->parent[k]) {
          prob *= LayoutActionProb(k);
        }
        return prob;
      },
      derivs);

  const size_t length = m_probs.size();
  p_derivs = Matrix<T>(length, length);
  p_derivs = static_cast<T>(0);
  for (size_t a = 1; a <= layout.numActions; a++) {
    const int row = m_layoutProfileIndex[a];
    if (row < 0) {
      continue;
    }
    for (size_t b = 1; b <= layout.numActions; b++) {
      if (const int col = m_layoutProfileIndex[b]; col >= 0) {
        p_derivs(row, col) = derivs(a, b);
      }
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
  m_support.GetGame()->EnsureStrategies();
  return MixedStrategyProfile<T>(*this);
}

template class MixedBehaviorProfile<double>;
template class MixedBehaviorProfile<Rational>;

} // end namespace Gambit
