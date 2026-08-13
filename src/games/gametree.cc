//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/gametree.cc
// Implementation of extensive game representation
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
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "gambit.h"
#include "gametree.h"
#include "writer.h"

namespace Gambit {

//========================================================================
//                   class TreeMixedStrategyProfileRep
//========================================================================

//========================================================================
//                   TreeMixedStrategyProfileRep<T>
//========================================================================

template <class T>
TreeMixedStrategyProfileRep<T>::TreeMixedStrategyProfileRep(
    const MixedBehaviorProfile<T> &p_profile)
  : MixedStrategyProfileRep<T>(p_profile.GetGame())
{
}

template <class T>
std::unique_ptr<MixedStrategyProfileRep<T>> TreeMixedStrategyProfileRep<T>::Copy() const
{
  return std::make_unique<TreeMixedStrategyProfileRep>(*this);
}

template <class T> void TreeMixedStrategyProfileRep<T>::MakeBehavior() const
{
  if (m_mixedBehavior == nullptr) {
    m_mixedBehavior = std::make_shared<MixedBehaviorProfile<T>>(MixedStrategyProfile<T>(Copy()));
  }
}

template <class T> void TreeMixedStrategyProfileRep<T>::OnProfileChanged() const
{
  m_mixedBehavior = nullptr;
}

template <class T> T TreeMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  MakeBehavior();
  return m_mixedBehavior->GetPayoff(m_mixedBehavior->GetGame()->GetPlayer(pl));
}

template <class T>
T TreeMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy) const
{
  TreeMixedStrategyProfileRep tmp(*this);
  tmp.SetStrategy(strategy);
  return tmp.GetPayoff(pl);
}

template <class T>
T TreeMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy1,
                                                 const GameStrategy &strategy2) const
{
  if (strategy1->GetPlayer() == strategy2->GetPlayer()) {
    return static_cast<T>(0);
  }
  TreeMixedStrategyProfileRep tmp(*this);
  tmp.SetStrategy(strategy1);
  tmp.SetStrategy(strategy2);
  return tmp.GetPayoff(pl);
}

template class TreeMixedStrategyProfileRep<double>;
template class TreeMixedStrategyProfileRep<Rational>;

//========================================================================
//                     class GameActionRep
//========================================================================

bool GameActionRep::Precedes(const GameNode &n) const
{
  GameNode node = n;

  while (node != node->GetGame()->GetRoot()) {
    if (node->GetPriorAction().get() == this) {
      return true;
    }
    node = node->GetParent();
  }
  return false;
}

void GameTreeRep::DeleteAction(GameAction p_action)
{
  auto action = p_action.get();
  auto *infoset = action->m_infoset;
  if (infoset->m_game != this) {
    throw MismatchException();
  }
  if (infoset->m_actions.size() == 1) {
    throw UndefinedException();
  }

  IncrementVersion();
  auto where =
      std::find(infoset->m_actions.begin(), infoset->m_actions.end(), p_action.get_shared());
  auto offset = where - infoset->m_actions.begin();
  (*where)->Invalidate();
  infoset->m_actions.erase(where);
  if (infoset->m_player->IsChance()) {
    infoset->m_probs.erase(std::next(infoset->m_probs.begin(), offset));
    NormalizeChanceProbs(infoset);
  }
  infoset->RenumberActions();

  for (auto member : infoset->m_members) {
    auto it = std::next(member->m_children.begin(), offset);
    DeleteTree(*it);
    m_numNodes--;
    (*it)->Invalidate();
    member->m_children.erase(it);
  }
  ClearComputedValues();
  InvalidateTreeOrdering();
}

GameInfoset GameActionRep::GetInfoset() const { return m_infoset->shared_from_this(); }

//========================================================================
//                       class GameInfosetRep
//========================================================================

GameInfosetRep::GameInfosetRep(GameRep *p_efg, int p_number, GamePlayerRep *p_player,
                               int p_actions)
  : m_game(p_efg), m_number(p_number), m_player(p_player), m_actions(p_actions)
{
  std::generate(m_actions.begin(), m_actions.end(), [this, i = 1]() mutable {
    return std::make_shared<GameActionRep>(i++, "", this);
  });
  if (p_player->IsChance()) {
    m_probs = std::vector<Number>(m_actions.size());
    std::fill(m_probs.begin(), m_probs.end(), Rational(1, m_actions.size()));
  }
}

GameInfosetRep::~GameInfosetRep()
{
  std::for_each(m_actions.begin(), m_actions.end(),
                [](const std::shared_ptr<GameActionRep> &a) { a->Invalidate(); });
}

void GameTreeRep::SetPlayer(GameInfoset p_infoset, GamePlayer p_player)
{
  if (p_infoset->m_game != this || p_player->m_game != this) {
    throw MismatchException();
  }
  if (p_infoset->GetPlayer()->IsChance() || p_player->IsChance()) {
    throw UndefinedException();
  }
  if (p_infoset->GetPlayer() == p_player) {
    return;
  }

  const auto oldPlayer = p_infoset->GetPlayer().get();
  IncrementVersion();
  oldPlayer->m_infosets.erase(std::find(oldPlayer->m_infosets.begin(), oldPlayer->m_infosets.end(),
                                        p_infoset.get_shared()));
  p_infoset->m_player = p_player.get();
  p_player->m_infosets.push_back(p_infoset);

  ClearComputedValues();
  InvalidateTreeOrdering();
}

bool GameInfosetRep::Precedes(GameNode p_node) const
{
  auto node = p_node.get();
  while (node->m_parent) {
    if (node->m_infoset == this) {
      return true;
    }
    node = node->m_parent;
  }
  return false;
}

GameAction GameTreeRep::InsertAction(GameInfoset p_infoset, GameAction p_action /* =nullptr */)
{
  if (p_action && p_action->GetInfoset() != p_infoset) {
    throw MismatchException();
  }
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }

  IncrementVersion();
  auto where = (p_action) ? std::find(p_infoset->m_actions.begin(), p_infoset->m_actions.end(),
                                      p_action.get_shared())
                          : p_infoset->m_actions.end();
  auto offset = where - p_infoset->m_actions.begin();

  auto action = std::make_shared<GameActionRep>(offset + 1, "", p_infoset.get());
  p_infoset->m_actions.insert(where, action);
  if (p_infoset->m_player->IsChance()) {
    p_infoset->m_probs.insert(std::next(p_infoset->m_probs.cbegin(), offset), Number());
  }
  p_infoset->RenumberActions();
  for (const auto &member : p_infoset->m_members) {
    member->m_children.insert(std::next(member->m_children.cbegin(), offset),
                              std::make_shared<GameNodeRep>(this, member.get()));
  }

  m_numNodes += p_infoset->m_members.size();
  // m_numNonterminalNodes stays unchanged when an action is appended to an information set
  ClearComputedValues();
  InvalidateTreeOrdering();
  return action;
}

void GameTreeRep::RelabelActions(const GameInfoset &p_infoset,
                                 const std::map<std::string, std::string> &p_labels)
{
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }
  // Resolve each key to exactly one action at the information set.
  //
  // The ambiguous-match check below defends against a state that pygambit's own
  // callers can no longer produce (append_move and add_action both guarantee
  // unique, nonempty action labels, and file loading normalizes labels before
  // returning a game), but this is a public entry point on the base Game
  // interface, so a duplicate label reaching here from some other caller must
  // still be rejected rather than silently resolved to an arbitrary action.
  std::map<GameActionRep *, std::string> assignment;
  std::set<const GameActionRep *> relabeled;
  for (const auto &[old_label, new_label] : p_labels) {
    GameActionRep *match = nullptr;
    for (const auto &action : p_infoset->m_actions) {
      if (action->GetLabel() == old_label) {
        if (match) {
          throw ValueException("Action label '" + old_label +
                               "' is ambiguous at this information set");
        }
        match = action.get();
      }
    }
    if (!match) {
      throw ValueException("No action with label '" + old_label + "' at this information set");
    }
    assignment[match] = new_label;
    relabeled.insert(match);
  }
  // Replacement labels must be legal, unique against untouched actions, and pairwise distinct
  std::set<std::string> targets;
  for (const auto &[action, new_label] : assignment) {
    p_infoset->CheckActionLabel(new_label, relabeled);
    if (!targets.insert(new_label).second) {
      throw ValueException("Action label '" + new_label +
                           "' would be duplicated by the relabelling");
    }
  }
  for (const auto &[action, new_label] : assignment) {
    action->m_label = new_label;
  }
}

void GameTreeRep::SetActions(const GameInfoset &p_infoset,
                             const std::vector<std::string> &p_labels,
                             const std::vector<Number> &p_probs)
{
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }
  if (p_infoset->IsChanceInfoset()) {
    if (p_probs.empty()) {
      throw UndefinedException(
          "The actions of an event are set together with their probabilities");
    }
    if (p_labels.size() != p_probs.size()) {
      throw DimensionException(
          "The number of probabilities given must match the number of actions");
    }
    ValidateDistribution(p_probs);
  }
  else if (!p_probs.empty()) {
    throw UndefinedException("Probabilities can only be specified for the actions of an event");
  }
  if (p_labels.empty()) {
    throw ValueException("At least one action must be specified");
  }
  std::set<std::string> declared;
  for (const auto &label : p_labels) {
    if (label.empty()) {
      throw ValueException("Action label must not be empty");
    }
    CheckLabel(label);
    if (!declared.insert(label).second) {
      throw ValueException("Action label '" + label + "' appears more than once");
    }
  }

  std::map<std::string, int> current;
  for (const auto &action : p_infoset->m_actions) {
    if (!current.emplace(action->GetLabel(), action->GetNumber() - 1).second) {
      throw ValueException("Action label '" + action->GetLabel() +
                           "' is ambiguous at this information set");
    }
  }
  std::vector<int> source;
  source.reserve(p_labels.size());
  for (const auto &label : p_labels) {
    const auto it = current.find(label);
    source.push_back((it != current.end()) ? it->second : -1);
  }
  std::vector<int> dropped;
  for (const auto &action : p_infoset->m_actions) {
    if (declared.count(action->GetLabel()) == 0) {
      dropped.push_back(action->GetNumber() - 1);
    }
  }

  IncrementVersion();

  const auto members = p_infoset->m_members;
  for (const auto &member : members) {
    if (!member->IsValid()) {
      continue;
    }
    std::vector<std::shared_ptr<GameNodeRep>> newChildren;
    newChildren.reserve(p_labels.size());
    for (const int src : source) {
      if (src >= 0) {
        newChildren.push_back(member->m_children.at(src));
      }
      else {
        newChildren.push_back(std::make_shared<GameNodeRep>(this, member.get()));
        m_numNodes++;
      }
    }
    for (const int idx : dropped) {
      const auto child = member->m_children.at(idx);
      DeleteTree(child);
      m_numNodes--;
      child->Invalidate();
    }
    member->m_children = std::move(newChildren);
  }

  std::vector<std::shared_ptr<GameActionRep>> newActions;
  newActions.reserve(p_labels.size());
  for (size_t i = 0; i < p_labels.size(); i++) {
    if (source[i] >= 0) {
      newActions.push_back(p_infoset->m_actions[source[i]]);
    }
    else {
      newActions.push_back(
          std::make_shared<GameActionRep>(static_cast<int>(i) + 1, p_labels[i], p_infoset.get()));
    }
  }
  for (const int idx : dropped) {
    p_infoset->m_actions.at(idx)->Invalidate();
  }
  p_infoset->m_actions = std::move(newActions);
  p_infoset->RenumberActions();
  if (p_infoset->IsChanceInfoset()) {
    p_infoset->m_probs = p_probs;
  }
  ClearComputedValues();
  InvalidateTreeOrdering();
}

void GameTreeRep::RemoveMember(GameInfosetRep *p_infoset, GameNodeRep *p_node)
{
  IncrementVersion();
  p_infoset->m_members.erase(std::find(p_infoset->m_members.begin(), p_infoset->m_members.end(),
                                       p_node->shared_from_this()));
  if (p_infoset->m_members.empty()) {
    auto player = p_infoset->m_player;
    p_infoset->Invalidate();
    p_infoset->m_player->m_infosets.erase(std::find(
        player->m_infosets.begin(), player->m_infosets.end(), p_infoset->shared_from_this()));
  }
  InvalidateTreeOrdering();
}

void GameTreeRep::Reveal(GameInfoset p_atInfoset, GamePlayer p_player)
{
  IncrementVersion();
  for (const auto &action : p_atInfoset->m_actions) {
    auto infosets = p_player->m_infosets;
    for (const auto &infoset : infosets) {
      auto members = infoset->m_members;
      // This information set holds all members of information set
      // which follow 'action'.
      GameInfoset newiset = nullptr;
      for (const auto &member : members) {
        if (action->Precedes(member)) {
          if (!newiset) {
            newiset = LeaveInfoset(member);
          }
          else {
            SetInfoset(member, newiset);
          }
        }
      }
    }
  }

  ClearComputedValues();
  InvalidateTreeOrdering();
}

//========================================================================
//                      class GameSubgameRep
//========================================================================

GameSubgame GameSubgameRep::GetParent() const
{
  auto p = m_parent.lock();
  return p;
}

GameSubgameRep::SubgameCollection GameSubgameRep::GetChildren() const
{
  return SubgameCollection(std::const_pointer_cast<GameSubgameRep>(shared_from_this()),
                           &m_children);
}

GameSubgameRep::InfosetCollection GameSubgameRep::GetSubgameDifference() const
{
  return InfosetCollection(std::const_pointer_cast<GameSubgameRep>(shared_from_this()),
                           &m_subgameDifference);
}

//========================================================================
//                         class GameNodeRep
//========================================================================

GameNodeRep::GameNodeRep(GameRep *e, GameNodeRep *p) : m_game(e), m_parent(p) {}

GameNodeRep::~GameNodeRep()
{
  std::for_each(m_children.begin(), m_children.end(),
                [](const std::shared_ptr<GameNodeRep> &n) { n->Invalidate(); });
}

GameNode GameNodeRep::GetNextSibling() const
{
  if (!m_parent || m_parent->m_children.back().get() == this) {
    return nullptr;
  }
  return *std::next(
      std::find(m_parent->m_children.begin(), m_parent->m_children.end(), shared_from_this()));
}

GameNode GameNodeRep::GetPriorSibling() const
{
  if (!m_parent || m_parent->m_children.front().get() == this) {
    return nullptr;
  }
  return *std::prev(
      std::find(m_parent->m_children.begin(), m_parent->m_children.end(), shared_from_this()));
}

GameAction GameNodeRep::GetPriorAction() const
{
  if (!m_parent) {
    return nullptr;
  }
  for (const auto &action : m_parent->m_infoset->m_actions) {
    if (m_parent->GetChild(action).get() == this) {
      return action;
    }
  }
  return nullptr;
}

GameAction GameNodeRep::GetOwnPriorAction() const
{
  return m_game->GetOwnPriorAction(std::const_pointer_cast<GameNodeRep>(shared_from_this()));
}

std::set<GameAction> GameInfosetRep::GetOwnPriorActions() const
{
  return m_game->GetOwnPriorActions(std::const_pointer_cast<GameInfosetRep>(shared_from_this()));
}

void GameNodeRep::DeleteOutcome(GameOutcomeRep *outc)
{
  m_game->IncrementVersion();
  if (outc == m_outcome) {
    m_outcome = nullptr;
  }
  for (auto child : m_children) {
    child->DeleteOutcome(outc);
  }
}

void GameTreeRep::SetOutcome(const GameNode &p_node, const GameOutcome &p_outcome)
{
  if (p_node->m_game != this) {
    throw MismatchException();
  }
  if (p_outcome && p_outcome->m_game != this) {
    throw MismatchException();
  }
  if (const auto newOutcome = p_outcome.get_shared().get(); newOutcome != p_node->m_outcome) {
    p_node->m_outcome = newOutcome;
    IncrementVersion();
  }
}

bool GameNodeRep::IsSuccessorOf(GameNode p_node) const
{
  auto *n = const_cast<GameNodeRep *>(this);
  while (n && n != p_node.get()) {
    n = n->m_parent;
  }
  return (n == p_node.get());
}

bool GameNodeRep::IsSubgameRoot() const
{
  if (m_children.empty()) {
    return !GetParent();
  }

  auto *tree_game = static_cast<GameTreeRep *>(m_game);
  return tree_game->GetSubgameData().m_subgameByRoot.contains(const_cast<GameNodeRep *>(this));
}

bool GameNodeRep::IsStrategyReachable() const
{
  auto tree_game = static_cast<GameTreeRep *>(m_game);

  // A node is reachable if it is NOT in the set of unreachable nodes.
  return !tree_game->GetUnreachableNodes().contains(const_cast<GameNodeRep *>(this));
}

void GameTreeRep::DeleteParent(GameNode p_node)
{
  if (p_node->m_game != this) {
    throw MismatchException();
  }
  auto *node = p_node.get();
  if (!node->m_parent) {
    return;
  }
  IncrementVersion();
  auto *oldParent = node->m_parent;

  oldParent->m_children.erase(std::find(oldParent->m_children.begin(), oldParent->m_children.end(),
                                        node->shared_from_this()));
  DeleteTree(oldParent->shared_from_this());
  node->m_parent = oldParent->m_parent;
  m_numNodes--;
  if (node->m_parent) {
    std::replace(node->m_parent->m_children.begin(), node->m_parent->m_children.end(),
                 oldParent->shared_from_this(), node->shared_from_this());
  }
  else {
    m_root = p_node;
  }

  oldParent->Invalidate();
  ClearComputedValues();
  InvalidateTreeOrdering();
}

void GameTreeRep::DeleteTree(GameNode p_node)
{
  if (p_node->m_game != this) {
    throw MismatchException();
  }
  GameNodeRep *node = p_node.get();
  if (!node->IsTerminal()) {
    m_numNonterminalNodes--;
  }
  IncrementVersion();
  while (!node->m_children.empty()) {
    DeleteTree(node->m_children.back());
    m_numNodes--;
    node->m_children.back()->Invalidate();
    node->m_children.pop_back();
  }
  if (node->m_infoset) {
    RemoveMember(node->m_infoset, node);
    node->m_infoset = nullptr;
  }
  node->m_outcome = nullptr;
  node->m_label = "";

  ClearComputedValues();
  InvalidateTreeOrdering();
}

void GameTreeRep::CopySubtree(GameNodeRep *dest, GameNodeRep *src, GameNodeRep *stop)
{
  IncrementVersion();
  if (src == stop) {
    dest->m_outcome = src->m_outcome;
    return;
  }

  if (!src->m_children.empty()) {
    AppendMove(dest->shared_from_this(), src->m_infoset->shared_from_this());
    for (auto dest_child = dest->m_children.begin(), src_child = src->m_children.begin();
         src_child != src->m_children.end(); src_child++, dest_child++) {
      CopySubtree(dest_child->get(), src_child->get(), stop);
    }
  }

  dest->m_label = src->m_label;
  dest->m_outcome = src->m_outcome;
}

void GameTreeRep::CopyTree(GameNode p_dest, GameNode p_src)
{
  if (p_dest->m_game != this || p_src->m_game != this) {
    throw MismatchException();
  }
  GameNodeRep *dest = p_dest.get();
  GameNodeRep *src = p_src.get();
  if (dest == src || !dest->m_children.empty()) {
    return;
  }

  IncrementVersion();
  if (!src->m_children.empty()) {
    AppendMove(dest->shared_from_this(), src->m_infoset->shared_from_this());
    for (auto dest_child = dest->m_children.begin(), src_child = src->m_children.begin();
         src_child != src->m_children.end(); src_child++, dest_child++) {
      CopySubtree(dest_child->get(), src_child->get(), dest);
    }
    ClearComputedValues();
    InvalidateTreeOrdering();
  }
}

void GameTreeRep::MoveTree(GameNode p_dest, GameNode p_src)
{
  if (p_dest->m_game != this || p_src->m_game != this) {
    throw MismatchException();
  }
  GameNodeRep *dest = p_dest.get();
  GameNodeRep *src = p_src.get();
  if (src == dest || !dest->m_children.empty() || p_dest->IsSuccessorOf(p_src)) {
    return;
  }
  IncrementVersion();
  std::iter_swap(std::find(src->m_parent->m_children.begin(), src->m_parent->m_children.end(),
                           src->shared_from_this()),
                 std::find(dest->m_parent->m_children.begin(), dest->m_parent->m_children.end(),
                           dest->shared_from_this()));
  std::swap(src->m_parent, dest->m_parent);
  dest->m_label = "";
  dest->m_outcome = nullptr;

  ClearComputedValues();
  InvalidateTreeOrdering();
}

Game GameTreeRep::CopySubgame(GameNode p_root) const
{
  if (p_root->m_game != this) {
    throw MismatchException();
  }
  std::ostringstream os;
  WriteEfgFile(os, p_root);
  std::istringstream is(os.str());
  return ReadGame(is);
}

GameInfoset GameTreeRep::MakeInfoset(const std::vector<GameNode> &p_nodes,
                                     const GamePlayer &p_player, const std::string &p_label)
{
  if (p_nodes.empty()) {
    throw ValueException("At least one node must be specified");
  }
  if (p_player->m_game != this) {
    throw MismatchException();
  }
  if (p_player->IsChance()) {
    throw UndefinedException("The information set must belong to a personal player, not chance");
  }
  std::set<GameNodeRep *> selected;
  for (const auto &node : p_nodes) {
    if (node->m_game != this) {
      throw MismatchException();
    }
    if (node->IsTerminal()) {
      throw UndefinedException("All nodes must be decision nodes");
    }
    if (node->m_infoset->IsChanceInfoset()) {
      throw UndefinedException("All nodes must belong to a personal player, not chance");
    }
    if (!selected.insert(node.get()).second) {
      throw ValueException("Each node may be referenced only once");
    }
  }
  // The first member defines the action labels and their order of the new information set.
  const auto &reference = p_nodes.front()->m_infoset->m_actions;
  for (const auto &node : p_nodes) {
    const auto &actions = node->m_infoset->m_actions;
    if (!std::equal(
            actions.begin(), actions.end(), reference.begin(), reference.end(),
            [](const std::shared_ptr<GameActionRep> &a, const std::shared_ptr<GameActionRep> &b) {
              return a->GetLabel() == b->GetLabel();
            })) {
      throw ValueException(
          "All nodes must have the same actions, with the same labels in the same order");
    }
  }
  // Destroy an infoset all of whose members are absorbed; the label it holds can then be reused.
  std::set<const GameInfosetRep *> absorbed;
  if (!p_label.empty()) {
    for (const auto &infoset : p_player->m_infosets) {
      if (infoset->GetLabel() == p_label &&
          std::all_of(infoset->m_members.begin(), infoset->m_members.end(),
                      [&selected](const std::shared_ptr<GameNodeRep> &m) {
                        return selected.contains(m.get());
                      })) {
        absorbed.insert(infoset.get());
      }
    }
  }
  p_player->CheckInfosetLabel(p_label, absorbed);

  IncrementVersion();
  auto newInfoset = std::make_shared<GameInfosetRep>(this, p_player->m_infosets.size() + 1,
                                                     p_player.get(), reference.size());
  auto dest = newInfoset->m_actions.begin();
  for (const auto &action : reference) {
    (*dest)->m_label = action->GetLabel();
    ++dest;
  }
  p_player->m_infosets.push_back(newInfoset);
  for (const auto &node : p_nodes) {
    RemoveMember(node->m_infoset, node.get());
    newInfoset->m_members.push_back(node);
    node->m_infoset = newInfoset.get();
  }
  newInfoset->SetLabel(p_label);
  ClearComputedValues();
  InvalidateInfosetOrdering();
  return newInfoset;
}

void GameTreeRep::SetInfoset(GameNode p_node, GameInfoset p_infoset)
{
  if (p_node->m_game != this || p_infoset->m_game != this) {
    throw MismatchException();
  }
  GameNodeRep *node = p_node.get();
  if (!node->m_infoset || node->m_infoset == p_infoset.get()) {
    return;
  }
  if (p_infoset->m_actions.size() != node->m_children.size()) {
    throw DimensionException();
  }
  IncrementVersion();
  RemoveMember(node->m_infoset, node);
  p_infoset->m_members.push_back(p_node);
  node->m_infoset = p_infoset.get();

  ClearComputedValues();
  InvalidateInfosetOrdering();
}

GameInfoset GameTreeRep::LeaveInfoset(GameNode p_node)
{
  GameNodeRep *node = p_node.get();
  if (node->m_game != this) {
    throw MismatchException();
  }
  if (!node->m_infoset) {
    return nullptr;
  }

  IncrementVersion();
  auto *oldInfoset = node->m_infoset;
  if (oldInfoset->m_members.size() == 1) {
    return oldInfoset->shared_from_this();
  }

  GamePlayerRep *player = oldInfoset->m_player;
  RemoveMember(oldInfoset, node);
  auto newInfoset = std::make_shared<GameInfosetRep>(this, player->m_infosets.size() + 1, player,
                                                     node->m_children.size());
  player->m_infosets.push_back(newInfoset);
  node->m_infoset = newInfoset.get();
  node->m_infoset->m_members.push_back(p_node);
  for (auto old_act = oldInfoset->m_actions.begin(), new_act = node->m_infoset->m_actions.begin();
       old_act != oldInfoset->m_actions.end(); ++old_act, ++new_act) {
    (*new_act)->m_label = (*old_act)->GetLabel();
  }
  ClearComputedValues();
  InvalidateInfosetOrdering();
  return node->m_infoset->shared_from_this();
}

GameInfoset GameTreeRep::AppendMove(GameNode p_node, GamePlayer p_player,
                                    const std::vector<std::string> &p_actions)
{
  const GameNodeRep *node = p_node.get();
  if (p_actions.empty() || !node->m_children.empty()) {
    throw UndefinedException();
  }
  if (p_node->m_game != this || p_player->m_game != this) {
    throw MismatchException();
  }
  for (const auto &label : p_actions) {
    CheckLabel(label);
  }

  IncrementVersion();
  auto newInfoset = std::make_shared<GameInfosetRep>(
      this, p_player->m_infosets.size() + 1, p_player.get(), static_cast<int>(p_actions.size()));
  p_player->m_infosets.push_back(newInfoset);
  auto label_it = p_actions.begin();
  for (const auto &action : newInfoset->m_actions) {
    action->m_label = *label_it;
    ++label_it;
  }
  return AppendMove(p_node, newInfoset);
}

GameInfoset GameTreeRep::AppendMove(GameNode p_node, GameInfoset p_infoset)
{
  GameNodeRep *node = p_node.get();
  if (!node->m_children.empty()) {
    throw UndefinedException();
  }
  if (p_node->m_game != this || p_infoset->m_game != this) {
    throw MismatchException();
  }

  IncrementVersion();
  node->m_infoset = p_infoset.get();
  node->m_infoset->m_members.push_back(p_node);
  std::for_each(node->m_infoset->m_actions.begin(), node->m_infoset->m_actions.end(),
                [this, node](const std::shared_ptr<GameActionRep> &) {
                  node->m_children.push_back(std::make_shared<GameNodeRep>(this, node));
                  m_numNodes++;
                });
  m_numNonterminalNodes++;
  ClearComputedValues();
  InvalidateTreeOrdering();
  return node->m_infoset->shared_from_this();
}

GameInfoset GameTreeRep::InsertMove(GameNode p_node, GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0) {
    throw UndefinedException();
  }
  if (p_player->m_game != this) {
    throw MismatchException();
  }

  IncrementVersion();
  auto newInfoset = std::make_shared<GameInfosetRep>(this, p_player->m_infosets.size() + 1,
                                                     p_player.get(), p_actions);
  p_player->m_infosets.push_back(newInfoset);
  std::for_each(newInfoset->m_actions.begin(), newInfoset->m_actions.end(),
                [act = 1](const GameAction &a) mutable { a->m_label = std::to_string(act++); });
  return InsertMove(p_node, newInfoset);
}

GameInfoset GameTreeRep::InsertMove(GameNode p_node, GamePlayer p_player,
                                    const std::vector<std::string> &p_actions)
{
  if (p_actions.empty()) {
    throw UndefinedException();
  }
  if (p_player->m_game != this) {
    throw MismatchException();
  }
  for (const auto &label : p_actions) {
    CheckLabel(label);
  }

  IncrementVersion();
  auto newInfoset = std::make_shared<GameInfosetRep>(
      this, p_player->m_infosets.size() + 1, p_player.get(), static_cast<int>(p_actions.size()));
  p_player->m_infosets.push_back(newInfoset);
  auto label_it = p_actions.begin();
  for (const auto &action : newInfoset->m_actions) {
    action->m_label = *label_it;
    ++label_it;
  }
  return InsertMove(p_node, newInfoset);
}

GameInfoset GameTreeRep::InsertMove(GameNode p_node, GameInfoset p_infoset)
{
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }

  IncrementVersion();
  GameNodeRep *node = p_node.get();
  auto newNode = std::make_shared<GameNodeRep>(this, node->m_parent);
  newNode->m_infoset = p_infoset.get();
  p_infoset->m_members.push_back(newNode);

  if (node->m_parent) {
    std::replace(node->m_parent->m_children.begin(), node->m_parent->m_children.end(),
                 node->shared_from_this(), newNode);
  }
  else {
    m_root = newNode;
  }

  node->m_parent = newNode.get();
  newNode->m_children.push_back(p_node);
  std::for_each(
      std::next(newNode->m_infoset->m_actions.begin()), newNode->m_infoset->m_actions.end(),
      [this, newNode](const std::shared_ptr<GameActionRep> &) {
        newNode->m_children.push_back(std::make_shared<GameNodeRep>(this, newNode.get()));
      });

  // Total nodes added = 1 (newNode) + (NumActions - 1) (new children of newNode) = NumActions
  m_numNodes += newNode->m_infoset->m_actions.size();
  m_numNonterminalNodes++;
  ClearComputedValues();
  InvalidateTreeOrdering();
  return p_infoset;
}

//========================================================================
//                           class GameTreeRep
//========================================================================

//------------------------------------------------------------------------
//                        GameTreeRep: Lifecycle
//------------------------------------------------------------------------

GameTreeRep::GameTreeRep()
  : m_root(std::make_shared<GameNodeRep>(this, nullptr)),
    m_chance(std::make_shared<GamePlayerRep>(this, 0, "Chance"))
{
}

GameTreeRep::~GameTreeRep()
{
  m_root->Invalidate();
  m_chance->Invalidate();
}

Game GameTreeRep::Copy() const
{
  std::ostringstream os;
  WriteEfgFile(os);
  std::istringstream is(os.str());
  return ReadGame(is);
}

Game NewTree() { return std::make_shared<GameTreeRep>(); }

//------------------------------------------------------------------------
//                 GameTreeRep: General data access
//------------------------------------------------------------------------

bool GameTreeRep::IsConstSum() const
{
  struct ConstSumCallback {
    const GameTreeRep *m_game;
    std::map<GameNode, Rational> m_subtreeSums;
    bool m_isConstSum{true};

    static DFSCallbackResult OnEnter(GameNode, int) { return DFSCallbackResult::Continue; }
    static DFSCallbackResult OnAction(GameNode, GameNode, int)
    {
      return DFSCallbackResult::Continue;
    }
    DFSCallbackResult OnExit(const GameNode &p_node, int)
    {
      Rational sum(0);
      if (!p_node->IsTerminal()) {
        const auto children = p_node->GetChildren();

        if (std::adjacent_find(children.begin(), children.end(),
                               [&](const GameNode &a, const GameNode &b) {
                                 return m_subtreeSums[a] != m_subtreeSums[b];
                               }) != children.end()) {
          m_isConstSum = false;
          return DFSCallbackResult::Stop;
        }
        sum = m_subtreeSums[*children.begin()];
        for (const auto &child : children) {
          m_subtreeSums.erase(child);
        }
      }

      if (const auto outcome = p_node->GetOutcome()) {
        sum += sum_function(m_game->m_players, [&](const auto &p_player) {
          return outcome->GetPayoff<Rational>(p_player);
        });
      }
      m_subtreeSums[p_node] = sum;
      return DFSCallbackResult::Continue;
    }
    static void OnVisit(GameNode, int) {}
  };

  ConstSumCallback callback{this};
  WalkDFS(Game(const_cast<GameTreeRep *>(this)->shared_from_this()), m_root,
          TraversalOrder::Postorder, callback);
  return callback.m_isConstSum;
}

template <class Aggregator>
Rational GameTreeRep::AggregateSubtreePayoff(const GamePlayer &p_player,
                                             Aggregator p_aggregator) const
{
  struct AggregatePayoffCallback {
    const GamePlayer &m_player;
    Aggregator m_aggregator;

    std::map<GameNode, Rational> m_subtreeValues;
    Rational m_result{0};

    static DFSCallbackResult OnEnter(GameNode, int) { return DFSCallbackResult::Continue; }
    static DFSCallbackResult OnAction(GameNode, GameNode, int)
    {
      return DFSCallbackResult::Continue;
    }
    DFSCallbackResult OnExit(const GameNode &p_node, int)
    {
      Rational value(0);
      if (!p_node->IsTerminal()) {
        const auto children = p_node->GetChildren();
        value = m_aggregator(children, [&](const GameNode &c) { return m_subtreeValues[c]; });
        for (const auto &child : children) {
          m_subtreeValues.erase(child);
        }
      }
      if (const auto outcome = p_node->GetOutcome()) {
        value += outcome->GetPayoff<Rational>(m_player);
      }
      m_subtreeValues[p_node] = value;
      m_result = value; // We write the root node value last, so will be correct on termination
      return DFSCallbackResult::Continue;
    }

    static void OnVisit(GameNode, int) {}
  };

  AggregatePayoffCallback callback{p_player, std::move(p_aggregator)};

  WalkDFS(Game(const_cast<GameTreeRep *>(this)->shared_from_this()), m_root,
          TraversalOrder::Postorder, callback);

  return callback.m_result;
}

Rational GameTreeRep::GetPlayerMinPayoff(const GamePlayer &p_player) const
{
  return AggregateSubtreePayoff(p_player, [](const auto &range, auto value_fn) {
    return minimize_function(range, value_fn);
  });
}

Rational GameTreeRep::GetPlayerMaxPayoff(const GamePlayer &p_player) const
{
  return AggregateSubtreePayoff(p_player, [](const auto &range, auto value_fn) {
    return maximize_function(range, value_fn);
  });
}

bool GameTreeRep::IsPerfectRecall() const
{
  EnsureOwnPriorActions();

  if (GetRoot()->IsTerminal()) {
    return true;
  }

  return std::all_of(m_ownPriorActionInfo->infoset_map.cbegin(),
                     m_ownPriorActionInfo->infoset_map.cend(),
                     [](const auto &pair) { return pair.second.size() <= 1; });
}

bool GameTreeRep::IsAbsentMinded(const GameInfoset &p_infoset) const
{
  if (p_infoset->GetGame().get() != this) {
    throw MismatchException();
  }
  EnsureOwnPriorActions();
  return m_absentMindedInfosets.contains(p_infoset.get());
}

GameSubgame GameTreeRep::GetMinimalSubgame(const GameInfoset &p_infoset) const
{
  if (p_infoset->GetGame().get() != this) {
    throw MismatchException();
  }
  const auto &subgameData = GetSubgameData();
  auto *n = p_infoset->m_members.front().get();
  auto it = subgameData.m_subgameByRoot.find(n);
  while (it == subgameData.m_subgameByRoot.end()) {
    n = n->m_parent;
    it = subgameData.m_subgameByRoot.find(n);
  }
  return it->second;
}

std::vector<std::pair<GameInfoset, GameNode>> GameTreeRep::GetAbsentMindedReentries() const
{
  EnsureOwnPriorActions();
  if (m_absentMindedReentries.empty()) {
    return {};
  }

  std::vector<std::pair<GameInfoset, GameNode>> result;
  result.reserve(m_absentMindedReentries.size());
  for (const auto &[infoset, node] : m_absentMindedReentries) {
    result.emplace_back(infoset->shared_from_this(), node->shared_from_this());
  }
  return result;
}

//------------------------------------------------------------------------
//               GameTreeRep: Managing the representation
//------------------------------------------------------------------------

void GameTreeRep::SortInfosets(GamePlayerRep *p_player)
{
  // Sort nodes within information sets according to ID.
  for (auto &infoset : p_player->m_infosets) {
    std::sort(infoset->m_members.begin(), infoset->m_members.end(),
              [](const std::shared_ptr<GameNodeRep> &a, const std::shared_ptr<GameNodeRep> &b) {
                return a->m_number < b->m_number;
              });
  }
  // Sort information sets by the smallest ID among their members
  std::sort(
      p_player->m_infosets.begin(), p_player->m_infosets.end(),
      [](const std::shared_ptr<GameInfosetRep> &a, const std::shared_ptr<GameInfosetRep> &b) {
        return a->m_members.front()->m_number < b->m_members.front()->m_number;
      });
  RenumberInfosets(p_player);
}

void GameTreeRep::RenumberInfosets(GamePlayerRep *p_player)
{
  std::for_each(
      p_player->m_infosets.begin(), p_player->m_infosets.end(),
      [iset = 1](const std::shared_ptr<GameInfosetRep> &s) mutable { s->m_number = iset++; });
}

void GameTreeRep::EnsureNodeOrdering() const
{
  m_nodeOrdering.Ensure([&] {
    int nodeindex = 1;
    for (const auto &node : GetNodes()) {
      node->m_number = nodeindex++;
    }
  });
}

void GameTreeRep::EnsureInfosetOrdering() const
{
  m_infosetOrdering.Ensure([&] {
    EnsureNodeOrdering();
    for (auto player : GetPlayersWithChance()) {
      SortInfosets(player.get());
    }
  });
}

void GameTreeRep::ClearComputedValues() const
{
  for (auto player : m_players) {
    for (auto strategy : player->m_strategies) {
      strategy->Invalidate();
    }
    player->m_strategies.clear();
    for (const auto &sequence : player->m_sequences) {
      sequence->Invalidate();
    }
    player->m_sequences.clear();
  }
  m_sequences.Invalidate();
  const_cast<GameTreeRep *>(this)->m_nodePlays.clear();
  m_ownPriorActions.Invalidate();
  m_unreachableNodes.Invalidate();
  m_absentMindedInfosets.clear();
  m_subgameData.IfBuilt([](const SubgameData &sd) {
    for (const auto &[node, subgame] : sd.m_subgameByRoot) {
      subgame->Invalidate();
    }
  });
  m_subgameData.Invalidate();
  m_absentMindedReentries.clear();
  m_strategies.Invalidate();
}

void GameTreeRep::EnsureStrategies() const
{
  m_strategies.Ensure([&] {
    EnsureInfosetOrdering();
    for (const auto &player : m_players) {
      std::map<GameInfosetRep *, int> behav;
      std::map<GameNodeRep *, GameNodeRep *> ptr, whichbranch;
      player->MakeReducedStrats(m_root.get(), nullptr, behav, ptr, whichbranch);
    }
    IndexStrategies();
  });
}

void GameTreeRep::BuildSequences(const GameNode &n, PureSequenceProfile &p_currentSequences) const
{
  if (!n->GetInfoset()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (auto child : n->GetChildren()) {
      BuildSequences(child, p_currentSequences);
    }
  }
  else {
    auto *player = n->m_infoset->m_player;
    const auto tmp_sequence = p_currentSequences.GetSequence(n->GetPlayer());
    for (const auto &action : n->m_infoset->m_actions) {
      auto seq_it =
          std::find_if(player->m_sequences.begin(), player->m_sequences.end(),
                       [&action](const auto seq) { return seq->m_action == action.get(); });
      std::shared_ptr<GameSequenceRep> sequence;
      if (seq_it == player->m_sequences.end()) {
        player->m_sequences.emplace_back(std::make_shared<GameSequenceRep>(
            n->m_infoset->m_player, action.get(), player->m_sequences.size() + 1,
            tmp_sequence.get_shared()));
        sequence = player->m_sequences.back();
      }
      else {
        sequence = *seq_it;
      }
      p_currentSequences.SetSequence(sequence);
      BuildSequences(n->GetChild(action), p_currentSequences);
    }
    p_currentSequences.SetSequence(tmp_sequence);
  }
}

void GameTreeRep::EnsureSequences() const
{
  m_sequences.Ensure([&] {
    PureSequenceProfile currentSequences(m_root->GetGame());
    for (const auto &player : m_players) {
      player->m_sequences = {std::make_shared<GameSequenceRep>(player.get(), nullptr, 1,
                                                               std::weak_ptr<GameSequenceRep>())};
      currentSequences.SetSequence(player->m_sequences.front());
    }
    BuildSequences(m_root, currentSequences);
  });
}

void GameTreeRep::BuildConsistentPlays()
{
  m_nodePlays.clear();
  BuildConsistentPlaysRecursiveImpl(m_root.get());
}

std::vector<GameNodeRep *> GameTreeRep::BuildConsistentPlaysRecursiveImpl(GameNodeRep *node)
{
  std::vector<GameNodeRep *> consistent_plays;
  if (node->IsTerminal()) {
    consistent_plays = std::vector<GameNodeRep *>{node};
  }
  else {
    for (auto child : node->GetChildren()) {
      auto child_consistent_plays = BuildConsistentPlaysRecursiveImpl(child.get());
      consistent_plays.insert(consistent_plays.end(), child_consistent_plays.begin(),
                              child_consistent_plays.end());
    }
  }
  m_nodePlays[node] = consistent_plays;
  return consistent_plays;
}

void GameTreeRep::EnsureOwnPriorActions() const
{
  m_ownPriorActions.Ensure([&] {
    if (m_root->IsTerminal()) {
      m_ownPriorActionInfo = std::make_shared<OwnPriorActionInfo>();
      m_absentMindedInfosets.clear();
      m_absentMindedReentries.clear();
      return;
    }

    struct OwnPriorActionsVisitor {
      std::shared_ptr<OwnPriorActionInfo> m_info;
      std::map<GamePlayer, std::stack<GameAction>> m_priorActions;

      // A node is a re-entry of its information set iff an ancestor on the current
      // root-to-node path shares that information set.  m_pathMemberCount counts, per information
      // set, how many nodes on the current path belong to it.
      std::map<GameInfosetRep *, int> m_pathMemberCount;
      std::set<GameInfosetRep *> m_absentMindedInfosets;
      std::vector<std::pair<GameInfosetRep *, GameNodeRep *>> m_absentMindedReentries;

      explicit OwnPriorActionsVisitor(const GameTreeRep *p_game)
        : m_info(std::make_shared<OwnPriorActionInfo>())
      {
        for (const auto &player : p_game->GetPlayersWithChance()) {
          m_priorActions[player].emplace(nullptr);
        }
      }

      DFSCallbackResult OnEnter(GameNode p_node, int)
      {
        if (auto *infoset = p_node->m_infoset) {
          auto &stack = m_priorActions.at(infoset->m_player->shared_from_this());
          GameActionRep *raw_prior = stack.top() ? stack.top().get() : nullptr;

          m_info->node_map[p_node.get()] = raw_prior;
          m_info->infoset_map[infoset].insert(raw_prior);

          stack.emplace(nullptr);

          if (m_pathMemberCount[infoset]++ > 0) {
            m_absentMindedInfosets.insert(infoset);
            m_absentMindedReentries.emplace_back(infoset, p_node.get());
          }
        }
        return DFSCallbackResult::Continue;
      }

      DFSCallbackResult OnAction(GameNode p_parent, GameNode p_child, int)
      {
        m_priorActions.at(p_parent->m_infoset->m_player->shared_from_this()).top() =
            p_child->GetPriorAction();
        return DFSCallbackResult::Continue;
      }

      DFSCallbackResult OnExit(const GameNode &p_node, int)
      {
        if (auto *infoset = p_node->m_infoset) {
          m_priorActions.at(infoset->m_player->shared_from_this()).pop();
          m_pathMemberCount[infoset]--;
        }
        return DFSCallbackResult::Continue;
      }

      void OnVisit(GameNode, int) {}
    };

    OwnPriorActionsVisitor visitor(this);

    WalkDFS(const_cast<GameTreeRep *>(this)->shared_from_this(), m_root, TraversalOrder::Preorder,
            visitor);

    m_ownPriorActionInfo = visitor.m_info;
    m_absentMindedInfosets = std::move(visitor.m_absentMindedInfosets);
    m_absentMindedReentries = std::move(visitor.m_absentMindedReentries);
  });
}

GameAction GameTreeRep::GetOwnPriorAction(const GameNode &p_node) const
{
  EnsureOwnPriorActions();

  auto it = m_ownPriorActionInfo->node_map.find(p_node.get());
  if (it != m_ownPriorActionInfo->node_map.end() && it->second) {
    return it->second->shared_from_this();
  }
  return nullptr;
}

std::set<GameAction> GameTreeRep::GetOwnPriorActions(const GameInfoset &p_infoset) const
{
  EnsureOwnPriorActions();

  std::set<GameAction> result;
  auto it = m_ownPriorActionInfo->infoset_map.find(p_infoset.get());

  if (it != m_ownPriorActionInfo->infoset_map.end()) {
    for (auto *ptr : it->second) {
      result.insert(ptr ? ptr->shared_from_this() : nullptr);
    }
  }
  return result;
}

const std::set<GameNodeRep *> &GameTreeRep::GetUnreachableNodes() const
{
  return m_unreachableNodes.Get([&] {
    std::set<GameNodeRep *> result;

    if (m_root->IsTerminal()) {
      return result;
    }

    using AbsentMindedEdge = std::pair<GameAction, GameNode>;
    using ActiveEdge = std::variant<GameNodeRep::Actions::iterator, AbsentMindedEdge>;

    std::stack<ActiveEdge> position;
    std::map<GameInfoset, GameAction> path_choices;
    position.emplace(m_root->GetActions().begin());

    while (!position.empty()) {
      ActiveEdge &current_edge = position.top();
      GameNode child, node;
      GameAction action;

      if (std::holds_alternative<GameNodeRep::Actions::iterator>(current_edge)) {
        auto &current_it = std::get<GameNodeRep::Actions::iterator>(current_edge);
        node = current_it.GetOwner();

        if (current_it == node->GetActions().end()) {
          position.pop();
          path_choices.erase(node->m_infoset->shared_from_this());
          continue;
        }
        else {
          std::tie(action, child) = *current_it;
          ++current_it;
          path_choices[node->m_infoset->shared_from_this()] = action;
        }
      }
      else {
        std::tie(action, node) = std::get<AbsentMindedEdge>(current_edge);
        position.pop();
        child = node->GetChild(action);
      }

      if (!child->IsTerminal()) {
        // On a re-entry, a pure strategy replays the action chosen at the earlier visit,
        // so only that branch is reachable; prune the rest.
        if (path_choices.contains(child->m_infoset->shared_from_this())) {
          const GameAction replay_action = path_choices.at(child->m_infoset->shared_from_this());
          position.emplace(AbsentMindedEdge{replay_action, child});

          // Mark siblings and the nodes in their subtrees as unreachable
          for (const auto &[current_action, subtree_root] : child->GetActions()) {
            if (current_action != replay_action) {
              std::stack<GameNodeRep *> nodes_to_visit;
              nodes_to_visit.push(subtree_root.get());
              while (!nodes_to_visit.empty()) {
                // NOLINTBEGIN(misc-const-correctness)
                GameNodeRep *current_unreachable_node = nodes_to_visit.top();
                // NOLINTEND(misc-const-correctness)
                nodes_to_visit.pop();
                result.insert(current_unreachable_node);
                for (const auto &unreachable_child : current_unreachable_node->GetChildren()) {
                  nodes_to_visit.push(unreachable_child.get());
                }
              }
            }
          }
        }
        else {
          position.emplace(child->GetActions().begin());
        }
      }
    }

    return result;
  });
}

const GameTreeRep::SubgameData &GameTreeRep::GetSubgameData() const
{
  return m_subgameData.Get([&] {
    SubgameData sd;
    if (m_root->IsTerminal()) {
      return sd;
    }

    struct Range {
      int m_min = std::numeric_limits<int>::max();
      int m_max = 0;

      void Merge(const Range &p_source)
      {
        m_min = std::min(m_min, p_source.m_min);
        m_max = std::max(m_max, p_source.m_max);
      }

      bool operator==(const Range &p_other) const
      {
        return m_min == p_other.m_min && m_max == p_other.m_max;
      }
    };

    std::unordered_map<GameNodeRep *, Range> disc;
    std::unordered_map<GameInfosetRep *, Range> hull;

    // Phase 1: Compute subtree spans and infoset hulls
    struct SpanVisitor {
      std::unordered_map<GameNodeRep *, Range> &m_disc;
      std::unordered_map<GameInfosetRep *, Range> &m_hull;
      int m_counter = 0;

      static DFSCallbackResult OnEnter(GameNode, int) { return DFSCallbackResult::Continue; }
      static DFSCallbackResult OnAction(GameNode, GameNode, int)
      {
        return DFSCallbackResult::Continue;
      }
      static void OnVisit(GameNode, int) {}

      DFSCallbackResult OnExit(const GameNode &p_node, int)
      {
        GameNodeRep *node = p_node.get();
        if (p_node->IsTerminal()) {
          m_counter++;
          m_disc[node] = {m_counter, m_counter};
        }
        else {
          Range &node_disc = m_disc[node];
          const auto &children = p_node->GetChildren();
          node_disc.m_min = m_disc.at(children.front().get()).m_min;
          node_disc.m_max = m_disc.at(children.back().get()).m_max;
          m_hull[node->m_infoset].Merge(node_disc);
        }
        return DFSCallbackResult::Continue;
      }
    };

    // Phase 2: Reachability and detection
    struct BridgeVisitor {
      std::unordered_map<GameNodeRep *, Range> &m_disc;
      const std::unordered_map<GameInfosetRep *, Range> &m_hull;
      std::vector<GameNodeRep *> &m_subgames;
      std::unordered_map<GameNodeRep *, Range> m_low;

      static DFSCallbackResult OnEnter(GameNode, int) { return DFSCallbackResult::Continue; }
      static DFSCallbackResult OnAction(GameNode, GameNode, int)
      {
        return DFSCallbackResult::Continue;
      }
      static void OnVisit(GameNode, int) {}

      DFSCallbackResult OnExit(const GameNode &p_node, int)
      {
        GameNodeRep *node = p_node.get();
        if (p_node->IsTerminal()) {
          m_low[node] = m_disc.at(node);
          return DFSCallbackResult::Continue;
        }

        Range &low = m_low[node];
        low = m_hull.at(node->m_infoset);

        for (const auto &child : p_node->GetChildren()) {
          low.Merge(m_low.at(child.get()));
          m_low.erase(child.get());
        }

        if (low == m_disc.at(node)) {
          // The `low == disc` test is exact only with distinct terminal spans. A single-action
          // chain above a candidate node collapses the spans and can create false positives.
          // Reject a node if some single-action ancestor's infoset (possibly the node's own)
          // has a member in the node's subtree (possibly the node itself).
          // Note that such an infoset is necessarily absent-minded.
          bool spurious = false;
          for (auto *anc = node->m_parent; anc && anc->m_children.size() == 1 && !spurious;
               anc = anc->m_parent) {
            const auto &members = anc->m_infoset->m_members;
            spurious = members.size() >= 2 &&
                       std::any_of(members.begin(), members.end(), [&](const auto &member) {
                         return member.get() != anc && member->IsSuccessorOf(p_node);
                       });
          }
          if (!spurious) {
            m_subgames.push_back(node);
          }
        }

        return DFSCallbackResult::Continue;
      }
    };

    auto game = std::const_pointer_cast<GameRep>(shared_from_this());

    SpanVisitor span_visitor{disc, hull};
    WalkDFS(game, m_root, TraversalOrder::Postorder, span_visitor);

    BridgeVisitor bridge_visitor{disc, hull, sd.m_subgamePostorder};
    WalkDFS(game, m_root, TraversalOrder::Postorder, bridge_visitor);

    // Phase 3: Build subgame tree with subgame differences
    struct SubgameVisitor {
      const std::unordered_set<GameNodeRep *> &m_roots;
      std::unordered_map<GameNodeRep *, std::shared_ptr<GameSubgameRep>> &m_cache;
      GameTreeRep *m_game;
      // Subgame roots on the current DFS path, innermost at back
      std::vector<GameNodeRep *> m_stack;
      std::unordered_set<GameInfosetRep *> m_infosetVisited;

      DFSCallbackResult OnEnter(const GameNode &p_node, int)
      {
        if (p_node->IsTerminal()) {
          return DFSCallbackResult::Continue;
        }
        GameNodeRep *node = p_node.get();
        if (m_roots.contains(node)) {
          auto subgame = std::make_shared<GameSubgameRep>(m_game, node);
          if (!m_stack.empty()) {
            auto &parent_subgame = m_cache.at(m_stack.back());
            subgame->m_parent = parent_subgame;
            parent_subgame->m_children.push_back(subgame);
          }
          m_cache.emplace(node, std::move(subgame));
          m_stack.push_back(node);
        }
        if (m_infosetVisited.insert(node->m_infoset).second) {
          m_cache.at(m_stack.back())
              ->m_subgameDifference.emplace_back(node->m_infoset->shared_from_this());
        }
        return DFSCallbackResult::Continue;
      }

      DFSCallbackResult OnExit(const GameNode &p_node, int)
      {
        if (!m_stack.empty() && m_stack.back() == p_node.get()) {
          m_stack.pop_back();
        }
        return DFSCallbackResult::Continue;
      }

      static DFSCallbackResult OnAction(GameNode, GameNode, int)
      {
        return DFSCallbackResult::Continue;
      }
      static void OnVisit(GameNode, int) {}
    };

    const std::unordered_set<GameNodeRep *> subgame_root_set(sd.m_subgamePostorder.begin(),
                                                             sd.m_subgamePostorder.end());

    SubgameVisitor subgame_visitor{subgame_root_set, sd.m_subgameByRoot,
                                   const_cast<GameTreeRep *>(this)};
    WalkDFS(game, m_root, TraversalOrder::Preorder, subgame_visitor);
    return sd;
  });
}

std::vector<GameSubgame> GameTreeRep::GetSubgames() const
{
  const auto &subgameData = GetSubgameData();
  std::vector<GameSubgame> result;
  result.reserve(subgameData.m_subgamePostorder.size());
  for (auto *rep : subgameData.m_subgamePostorder) {
    result.emplace_back(subgameData.m_subgameByRoot.at(rep));
  }
  return result;
}

//------------------------------------------------------------------------
//                  GameTreeRep: Writing data files
//------------------------------------------------------------------------

namespace {

void WriteEfgFile(std::ostream &f, const GameNode &n)
{
  if (n->IsTerminal()) {
    f << "t ";
  }
  else if (n->GetInfoset()->IsChanceInfoset()) {
    f << "c ";
  }
  else {
    f << "p ";
  }
  f << QuoteString(n->GetLabel()) << ' ';
  if (!n->IsTerminal()) {
    if (!n->GetInfoset()->IsChanceInfoset()) {
      f << n->GetInfoset()->GetPlayer()->GetNumber() << ' ';
    }
    f << n->GetInfoset()->GetNumber() << " " << QuoteString(n->GetInfoset()->GetLabel()) << ' ';
    if (n->GetInfoset()->IsChanceInfoset()) {
      f << FormatList(n->GetInfoset()->GetActions(), [](const GameAction &a) {
        return QuoteString(a->GetLabel()) + " " + std::string(a->GetInfoset()->GetActionProb(a));
      });
    }
    else {
      f << FormatList(n->GetInfoset()->GetActions(),
                      [](const GameAction &a) { return QuoteString(a->GetLabel()); });
    }
    f << ' ';
  }
  if (n->GetOutcome()) {
    f << n->GetOutcome()->GetNumber() << " " << QuoteString(n->GetOutcome()->GetLabel()) << ' '
      << FormatList(
             n->GetGame()->GetPlayers(),
             [n](const GamePlayer &p) { return n->GetOutcome()->GetPayoff<std::string>(p); }, true)
      << std::endl;
  }
  else {
    f << "0" << std::endl;
  }
  for (auto child : n->GetChildren()) {
    WriteEfgFile(f, child);
  }
}

} // end anonymous namespace

void GameTreeRep::WriteEfgFile(std::ostream &p_file, const GameNode &p_subtree /* =0 */) const
{
  p_file << "EFG 2 R " << std::quoted(GetTitle()) << ' '
         << FormatList(GetPlayers(),
                       [](const GamePlayer &p) { return QuoteString(p->GetLabel()); })
         << std::endl;
  p_file << std::quoted(GetDescription()) << std::endl << std::endl;
  Gambit::WriteEfgFile(p_file, (p_subtree) ? p_subtree : GetRoot());
}

void GameTreeRep::WriteNfgFile(std::ostream &p_file) const
{
  EnsureStrategies();
  GameRep::WriteNfgFile(p_file);
}

//------------------------------------------------------------------------
//                 GameTreeRep: Dimensions of the game
//------------------------------------------------------------------------

int GameTreeRep::BehavProfileLength() const
{
  int sum = 0;
  for (auto player : m_players) {
    for (auto infoset : player->m_infosets) {
      sum += infoset->m_actions.size();
    }
  }
  return sum;
}

//------------------------------------------------------------------------
//                        GameTreeRep: Players
//------------------------------------------------------------------------

GamePlayer GameTreeRep::NewPlayer(const std::string &p_label)
{
  CheckPlayerLabel(p_label);
  auto player = std::make_shared<GamePlayerRep>(this, m_players.size() + 1, p_label);
  IncrementVersion();
  m_players.push_back(player);
  for (const auto &outcome : m_outcomes) {
    outcome->m_payoffs[player.get()] = Number();
  }
  ClearComputedValues();
  return player;
}

//------------------------------------------------------------------------
//                   GameTreeRep: Information sets
//------------------------------------------------------------------------

GameInfoset GameTreeRep::GetInfoset(int p_index) const
{
  int index = 1;
  for (auto player : m_players) {
    for (auto infoset : player->m_infosets) {
      if (index++ == p_index) {
        return infoset;
      }
    }
  }
  throw std::out_of_range("Infoset index out of range");
}

//------------------------------------------------------------------------
//                        GameTreeRep: Outcomes
//------------------------------------------------------------------------

std::vector<GameNode> GameTreeRep::GetPlays(GameNode node) const
{
  const_cast<GameTreeRep *>(this)->BuildConsistentPlays();

  const std::vector<GameNodeRep *> &consistent_plays = m_nodePlays.at(node.get());
  std::vector<GameNode> consistent_plays_copy;
  consistent_plays_copy.reserve(consistent_plays.size());

  std::transform(consistent_plays.cbegin(), consistent_plays.cend(),
                 std::back_inserter(consistent_plays_copy),
                 [](GameNodeRep *rep_ptr) -> GameNode { return {rep_ptr->shared_from_this()}; });

  return consistent_plays_copy;
}

std::vector<GameNode> GameTreeRep::GetPlays(GameInfoset infoset) const
{
  std::vector<GameNode> plays;

  for (const auto &node : infoset->GetMembers()) {
    std::vector<GameNode> member_plays = GetPlays(node);
    plays.insert(plays.end(), member_plays.begin(), member_plays.end());
  }
  return plays;
}

std::vector<GameNode> GameTreeRep::GetPlays(GameAction action) const
{
  std::vector<GameNode> plays;

  for (const auto &node : action->GetInfoset()->GetMembers()) {
    std::vector<GameNode> child_plays = GetPlays(node->GetChild(action));
    plays.insert(plays.end(), child_plays.begin(), child_plays.end());
  }
  return plays;
}

void GameTreeRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  IncrementVersion();
  m_root->DeleteOutcome(p_outcome.get());
  p_outcome->Invalidate();
  m_outcomes.erase(
      std::find(m_outcomes.begin(), m_outcomes.end(), std::shared_ptr<GameOutcomeRep>(p_outcome)));
  std::for_each(
      m_outcomes.begin(), m_outcomes.end(),
      [outc = 1](const std::shared_ptr<GameOutcomeRep> &c) mutable { c->m_number = outc++; });
}

//------------------------------------------------------------------------
//                       GameTreeRep: Modification
//------------------------------------------------------------------------

Game GameTreeRep::SetChanceProbs(const GameInfoset &p_infoset, const Array<Number> &p_probs)
{
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }
  if (!p_infoset->IsChanceInfoset()) {
    throw UndefinedException("Action probabilities can only be specified for events");
  }
  if (p_infoset->m_actions.size() != p_probs.size()) {
    throw DimensionException("The number of probabilities given must match the number of actions");
  }
  ValidateDistribution(p_probs);
  IncrementVersion();
  std::copy(p_probs.begin(), p_probs.end(), p_infoset->m_probs.begin());
  return shared_from_this();
}

GameInfoset GameTreeRep::MakeEvent(const std::vector<GameNode> &p_nodes,
                                   const std::vector<Number> &p_probs, const std::string &p_label)
{
  if (p_nodes.empty()) {
    throw ValueException("At least one node must be specified");
  }
  std::set<GameNodeRep *> selected;
  for (const auto &node : p_nodes) {
    if (node->m_game != this) {
      throw MismatchException();
    }
    if (node->IsTerminal()) {
      throw UndefinedException("All nodes must be nonterminal");
    }
    if (!selected.insert(node.get()).second) {
      throw ValueException("Each node may be referenced only once");
    }
  }
  // The first member defines the action labels and their order of the new event.
  const auto &reference = p_nodes.front()->m_infoset->m_actions;
  for (const auto &node : p_nodes) {
    const auto &actions = node->m_infoset->m_actions;
    if (!std::equal(
            actions.begin(), actions.end(), reference.begin(), reference.end(),
            [](const std::shared_ptr<GameActionRep> &a, const std::shared_ptr<GameActionRep> &b) {
              return a->GetLabel() == b->GetLabel();
            })) {
      throw ValueException(
          "All nodes must have the same actions, with the same labels in the same order");
    }
  }
  if (p_probs.size() != reference.size()) {
    throw DimensionException("The number of probabilities given must match the number of actions");
  }
  ValidateDistribution(p_probs);
  const GamePlayer chance = GetChance();
  // Destroy an event all of whose members are absorbed; its label can then be reused.
  std::set<const GameInfosetRep *> absorbed;
  if (!p_label.empty()) {
    for (const auto &infoset : chance->m_infosets) {
      if (infoset->GetLabel() == p_label &&
          std::all_of(infoset->m_members.begin(), infoset->m_members.end(),
                      [&selected](const std::shared_ptr<GameNodeRep> &m) {
                        return contains(selected, m.get());
                      })) {
        absorbed.insert(infoset.get());
      }
    }
  }
  chance->CheckInfosetLabel(p_label, absorbed);

  IncrementVersion();
  auto newEvent = std::make_shared<GameInfosetRep>(this, chance->m_infosets.size() + 1,
                                                   chance.get(), reference.size());
  auto dest = newEvent->m_actions.begin();
  for (const auto &action : reference) {
    (*dest)->m_label = action->m_label;
    ++dest;
  }
  std::copy(p_probs.begin(), p_probs.end(), newEvent->m_probs.begin());
  chance->m_infosets.push_back(newEvent);
  for (const auto &node : p_nodes) {
    RemoveMember(node->m_infoset, node.get());
    newEvent->m_members.push_back(node);
    node->m_infoset = newEvent.get();
  }
  newEvent->SetLabel(p_label);
  ClearComputedValues();
  InvalidateInfosetOrdering();
  return newEvent;
}

Game GameTreeRep::NormalizeChanceProbs(GameInfosetRep *p_infoset)
{
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }
  if (!p_infoset->IsChanceInfoset()) {
    throw UndefinedException("Action probabilities can only be normalized for eventss");
  }
  IncrementVersion();
  auto &probs = p_infoset->m_probs;
  auto sum = std::accumulate(
      probs.begin(), probs.end(), Rational(0),
      [](const Rational &s, const Number &n) { return s + static_cast<Rational>(n); });
  if (sum == Rational(0)) {
    // all remaining moves have prob zero; split prob 1 equally among them
    std::fill(probs.begin(), probs.end(), Rational(1, probs.size()));
  }
  else {
    std::transform(probs.begin(), probs.end(), probs.begin(),
                   [&sum](const Number &n) { return static_cast<Rational>(n) / sum; });
  }
  return shared_from_this();
}

//------------------------------------------------------------------------
//                     GameTreeRep: Factory functions
//------------------------------------------------------------------------

MixedStrategyProfile<double> GameTreeRep::NewMixedStrategyProfile(double) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureStrategies();
  return StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))
      .NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTreeRep::NewMixedStrategyProfile(const Rational &) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureStrategies();
  return StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))
      .NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double>
GameTreeRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureStrategies();
  return MixedStrategyProfile<double>(std::make_unique<TreeMixedStrategyProfileRep<double>>(spt));
}

MixedStrategyProfile<Rational>
GameTreeRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureStrategies();
  return MixedStrategyProfile<Rational>(
      std::make_unique<TreeMixedStrategyProfileRep<Rational>>(spt));
}

//========================================================================
//                  class TreePureStrategyProfileRep
//========================================================================

class TreePureStrategyProfileRep : public PureStrategyProfileRep {
protected:
  std::shared_ptr<PureStrategyProfileRep> Copy() const override
  {
    return std::make_shared<TreePureStrategyProfileRep>(*this);
  }

public:
  TreePureStrategyProfileRep(const Game &p_game) : PureStrategyProfileRep(p_game) {}
  GameOutcome GetOutcome() const override { throw UndefinedException(); }
  void SetOutcome(GameOutcome p_outcome) override { throw UndefinedException(); }
  Rational GetPayoff(const GamePlayer &) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//              TreePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

PureStrategyProfile GameTreeRep::NewPureStrategyProfile() const
{
  EnsureStrategies();
  return PureStrategyProfile(std::make_shared<TreePureStrategyProfileRep>(
      std::const_pointer_cast<GameRep>(shared_from_this())));
}

//------------------------------------------------------------------------
//       TreePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

Rational TreePureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  PureBehaviorProfile behav(m_game);
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      try {
        behav.SetAction(infoset->GetAction(GetStrategy(player)->m_behav.at(infoset.get())));
      }
      catch (std::out_of_range &) {
      }
    }
  }
  return behav.GetPayoff<Rational>(p_player);
}

Rational TreePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  const PureStrategyProfile copy(Copy());
  copy->SetStrategy(p_strategy);
  return copy->GetPayoff(p_strategy->GetPlayer());
}

} // end namespace Gambit
