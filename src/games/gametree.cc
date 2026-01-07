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

#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <stack>
#include <set>
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
  InvalidateNodeOrdering();
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
  InvalidateNodeOrdering();
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
  InvalidateNodeOrdering();
  return action;
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
    RenumberInfosets(player);
  }
  InvalidateNodeOrdering();
}

void GameTreeRep::Reveal(GameInfoset p_atInfoset, GamePlayer p_player)
{
  IncrementVersion();
  for (auto action : p_atInfoset->GetActions()) {
    for (auto infoset : p_player->m_infosets) {
      // make copy of members to iterate correctly
      // (since the information set may be changed in the process)
      auto members = infoset->m_members;

      // This information set holds all members of information set
      // which follow 'action'.
      GameInfoset newiset = nullptr;
      for (auto &member : members) {
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
  InvalidateNodeOrdering();
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
    ClearComputedValues();
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
  // First take care of a couple easy cases
  if (m_children.empty() || m_infoset->m_members.size() > 1) {
    return false;
  }
  if (!m_parent) {
    return true;
  }

  // A node is a subgame root if and only if in every information set,
  // either all members succeed the node in the tree,
  // or all members do not succeed the node in the tree.
  for (auto player : m_game->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      const bool precedes = infoset->m_members.front()->IsSuccessorOf(
          std::const_pointer_cast<GameNodeRep>(shared_from_this()));
      if (std::any_of(std::next(infoset->m_members.begin()), infoset->m_members.end(),
                      [this, precedes](const std::shared_ptr<GameNodeRep> &m) {
                        return m->IsSuccessorOf(std::const_pointer_cast<GameNodeRep>(
                                   shared_from_this())) != precedes;
                      })) {
        return false;
      }
    }
  }

  return true;
}

bool GameNodeRep::IsStrategyReachable() const
{
  auto tree_game = static_cast<GameTreeRep *>(m_game);

  if (!tree_game->m_unreachableNodes) {
    tree_game->BuildUnreachableNodes();
  }

  // A node is reachable if it is NOT in the set of unreachable nodes.
  return !contains(*tree_game->m_unreachableNodes, const_cast<GameNodeRep *>(this));
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
  InvalidateNodeOrdering();
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
  InvalidateNodeOrdering();
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
    InvalidateNodeOrdering();
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
  InvalidateNodeOrdering();
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
    (*new_act)->SetLabel((*old_act)->GetLabel());
  }
  ClearComputedValues();
  InvalidateInfosetOrdering();
  return node->m_infoset->shared_from_this();
}

GameInfoset GameTreeRep::AppendMove(GameNode p_node, GamePlayer p_player, int p_actions,
                                    bool p_generateLabels)
{
  const GameNodeRep *node = p_node.get();
  if (p_actions <= 0 || !node->m_children.empty()) {
    throw UndefinedException();
  }
  if (p_node->m_game != this || p_player->m_game != this) {
    throw MismatchException();
  }

  IncrementVersion();
  auto newInfoset = std::make_shared<GameInfosetRep>(this, p_player->m_infosets.size() + 1,
                                                     p_player.get(), p_actions);
  p_player->m_infosets.push_back(newInfoset);
  if (p_generateLabels) {
    std::for_each(newInfoset->m_actions.begin(), newInfoset->m_actions.end(),
                  [act = 1](const GameAction &a) mutable { a->SetLabel(std::to_string(act++)); });
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
  InvalidateNodeOrdering();
  return node->m_infoset->shared_from_this();
}

GameInfoset GameTreeRep::InsertMove(GameNode p_node, GamePlayer p_player, int p_actions,
                                    bool p_generateLabels)
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
  if (p_generateLabels) {
    std::for_each(newInfoset->m_actions.begin(), newInfoset->m_actions.end(),
                  [act = 1](const GameAction &a) mutable { a->SetLabel(std::to_string(act++)); });
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
  InvalidateNodeOrdering();
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
    m_chance(std::make_shared<GamePlayerRep>(this, 0))
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

namespace {

class NotZeroSumException final : public std::runtime_error {
public:
  NotZeroSumException() : std::runtime_error("Game is not constant sum") {}
  ~NotZeroSumException() noexcept override = default;
};

Rational SubtreeSum(GameNode p_node)
{
  Rational sum(0);

  if (!p_node->IsTerminal()) {
    const auto children = p_node->GetChildren();
    sum = SubtreeSum(children.front());
    if (std::any_of(std::next(children.begin()), children.end(),
                    [sum](const GameNode &n) { return SubtreeSum(n) != sum; })) {
      throw NotZeroSumException();
    }
  }

  if (p_node->GetOutcome()) {
    for (const auto &player : p_node->GetGame()->GetPlayers()) {
      sum += p_node->GetOutcome()->GetPayoff<Rational>(player);
    }
  }
  return sum;
}

Rational
AggregateSubtreePayoff(const GamePlayer &p_player, const GameNode &p_node,
                       std::function<Rational(const Rational &, const Rational &)> p_aggregator)
{
  if (p_node->IsTerminal()) {
    if (p_node->GetOutcome()) {
      return p_node->GetOutcome()->GetPayoff<Rational>(p_player);
    }
    return Rational(0);
  }
  const auto &children = p_node->GetChildren();
  auto subtree =
      std::accumulate(std::next(children.begin()), children.end(),
                      AggregateSubtreePayoff(p_player, children.front(), p_aggregator),
                      [&p_aggregator, &p_player](const Rational &r, const GameNode &c) {
                        return p_aggregator(r, AggregateSubtreePayoff(p_player, c, p_aggregator));
                      });
  if (p_node->GetOutcome()) {
    return subtree + p_node->GetOutcome()->GetPayoff<Rational>(p_player);
  }
  return subtree;
}

} // end anonymous namespace

bool GameTreeRep::IsConstSum() const
{
  try {
    SubtreeSum(m_root);
    return true;
  }
  catch (NotZeroSumException &) {
    return false;
  }
}

Rational GameTreeRep::GetPlayerMinPayoff(const GamePlayer &p_player) const
{
  return AggregateSubtreePayoff(
      p_player, m_root, [](const Rational &a, const Rational &b) { return std::min(a, b); });
}

Rational GameTreeRep::GetPlayerMaxPayoff(const GamePlayer &p_player) const
{
  return AggregateSubtreePayoff(
      p_player, m_root, [](const Rational &a, const Rational &b) { return std::max(a, b); });
}

bool GameTreeRep::IsPerfectRecall() const
{
  if (!m_ownPriorActionInfo && !m_root->IsTerminal()) {
    BuildOwnPriorActions();
  }

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

  if (!m_unreachableNodes && !m_root->IsTerminal()) {
    BuildUnreachableNodes();
  }

  return contains(m_absentMindedInfosets, p_infoset.get());
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
  if (m_nodesOrdered) {
    return;
  }
  int nodeindex = 1;
  for (const auto &node : GetNodes()) {
    node->m_number = nodeindex++;
  }
  m_nodesOrdered = true;
}

void GameTreeRep::EnsureInfosetOrdering() const
{
  if (m_infosetsOrdered) {
    return;
  }
  EnsureNodeOrdering();
  for (auto player : GetPlayersWithChance()) {
    SortInfosets(player.get());
  }
  m_infosetsOrdered = true;
}

void GameTreeRep::ClearComputedValues() const
{
  for (auto player : m_players) {
    for (auto strategy : player->m_strategies) {
      strategy->Invalidate();
    }
    player->m_strategies.clear();
  }
  const_cast<GameTreeRep *>(this)->m_nodePlays.clear();
  m_ownPriorActionInfo = nullptr;
  const_cast<GameTreeRep *>(this)->m_unreachableNodes = nullptr;
  m_absentMindedInfosets.clear();
  m_computedValues = false;
}

void GameTreeRep::BuildComputedValues() const
{
  if (m_computedValues) {
    return;
  }
  EnsureInfosetOrdering();
  for (const auto &player : m_players) {
    std::map<GameInfosetRep *, int> behav;
    std::map<GameNodeRep *, GameNodeRep *> ptr, whichbranch;
    player->MakeReducedStrats(m_root.get(), nullptr, behav, ptr, whichbranch);
  }
  m_computedValues = true;
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

void GameTreeRep::BuildOwnPriorActions() const
{
  auto info = std::make_shared<OwnPriorActionInfo>();

  if (m_root->IsTerminal()) {
    m_ownPriorActionInfo = info;
    return;
  }

  info->node_map[m_root.get()] = nullptr;
  if (m_root->m_infoset) {
    info->infoset_map[m_root->m_infoset].insert(nullptr);
  }

  using ActiveEdge = GameNodeRep::Actions::iterator;

  std::stack<ActiveEdge> position;
  std::map<GamePlayer, std::stack<GameAction>> prior_actions;

  for (auto player_rep : m_players) {
    prior_actions[GamePlayer(player_rep)].emplace(nullptr);
  }
  prior_actions[GamePlayer(m_chance)].emplace(nullptr);

  position.emplace(m_root->GetActions().begin());
  if (m_root->m_infoset) {
    prior_actions[m_root->m_infoset->m_player->shared_from_this()].emplace(nullptr);
  }

  while (!position.empty()) {
    ActiveEdge &current_edge = position.top();
    auto node = current_edge.GetOwner();

    if (current_edge == node->GetActions().end()) {
      if (node->m_infoset) {
        prior_actions.at(node->m_infoset->m_player->shared_from_this()).pop();
      }
      position.pop();
      continue;
    }

    auto [action, child] = *current_edge;
    ++current_edge;

    if (node->m_infoset) {
      prior_actions.at(node->m_infoset->m_player->shared_from_this()).top() = action;
    }

    if (!child->IsTerminal()) {
      if (child->m_infoset) {
        auto child_player = child->m_infoset->m_player->shared_from_this();
        auto prior_action = prior_actions.at(child_player).top();
        GameActionRep *raw_prior = prior_action ? prior_action.get() : nullptr;

        info->node_map[child.get()] = raw_prior;
        info->infoset_map[child->m_infoset].insert(raw_prior);

        position.emplace(child->GetActions().begin());
        prior_actions.at(child_player).emplace(nullptr);
      }
      else {
        position.emplace(child->GetActions().begin());
      }
    }
  }
  m_ownPriorActionInfo = info;
}

GameAction GameTreeRep::GetOwnPriorAction(const GameNode &p_node) const
{
  if (!m_ownPriorActionInfo) {
    BuildOwnPriorActions();
  }

  auto it = m_ownPriorActionInfo->node_map.find(p_node.get());
  if (it != m_ownPriorActionInfo->node_map.end() && it->second) {
    return it->second->shared_from_this();
  }
  return nullptr;
}

std::set<GameAction> GameTreeRep::GetOwnPriorActions(const GameInfoset &p_infoset) const
{
  if (!m_ownPriorActionInfo) {
    BuildOwnPriorActions();
  }

  std::set<GameAction> result;
  auto it = m_ownPriorActionInfo->infoset_map.find(p_infoset.get());

  if (it != m_ownPriorActionInfo->infoset_map.end()) {
    for (auto *ptr : it->second) {
      result.insert(ptr ? ptr->shared_from_this() : nullptr);
    }
  }
  return result;
}

void GameTreeRep::BuildUnreachableNodes() const
{
  m_unreachableNodes = std::make_unique<std::set<GameNodeRep *>>();

  if (m_root->IsTerminal()) {
    return;
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
      // Check for Absent-Minded Re-entry of the infoset
      if (path_choices.find(child->m_infoset->shared_from_this()) != path_choices.end()) {
        m_absentMindedInfosets.insert(child->m_infoset);
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
              m_unreachableNodes->insert(current_unreachable_node);
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
  p_file << std::quoted(GetComment()) << std::endl << std::endl;
  Gambit::WriteEfgFile(p_file, (p_subtree) ? p_subtree : GetRoot());
}

void GameTreeRep::WriteNfgFile(std::ostream &p_file) const
{
  BuildComputedValues();
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

GamePlayer GameTreeRep::NewPlayer()
{
  IncrementVersion();
  auto player = std::make_shared<GamePlayerRep>(this, m_players.size() + 1);
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
  ClearComputedValues();
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
    throw UndefinedException(
        "Action probabilities can only be specified for chance information sets");
  }
  if (p_infoset->m_actions.size() != p_probs.size()) {
    throw DimensionException("The number of probabilities given must match the number of actions");
  }
  IncrementVersion();
  if (std::any_of(p_probs.begin(), p_probs.end(),
                  [](const Number &x) { return static_cast<Rational>(x) < Rational(0); })) {
    throw ValueException("Probabilities must be non-negative numbers");
  }
  auto sum = std::accumulate(
      p_probs.begin(), p_probs.end(), Rational(0),
      [](const Rational &r, const Number &n) { return r + static_cast<Rational>(n); });
  if (sum != Rational(1)) {
    throw ValueException("Probabilities must sum to exactly one");
  }
  std::copy(p_probs.begin(), p_probs.end(), p_infoset->m_probs.begin());
  ClearComputedValues();
  return shared_from_this();
}

Game GameTreeRep::NormalizeChanceProbs(GameInfosetRep *p_infoset)
{
  if (p_infoset->m_game != this) {
    throw MismatchException();
  }
  if (!p_infoset->IsChanceInfoset()) {
    throw UndefinedException(
        "Action probabilities can only be normalized for chance information sets");
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
  EnsureInfosetOrdering();
  return StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))
      .NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTreeRep::NewMixedStrategyProfile(const Rational &) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureInfosetOrdering();
  return StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))
      .NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double>
GameTreeRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureInfosetOrdering();
  return MixedStrategyProfile<double>(std::make_unique<TreeMixedStrategyProfileRep<double>>(spt));
}

MixedStrategyProfile<Rational>
GameTreeRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  EnsureInfosetOrdering();
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
  EnsureInfosetOrdering();
  return PureStrategyProfile(std::make_shared<TreePureStrategyProfileRep>(
      std::const_pointer_cast<GameRep>(shared_from_this())));
}

//------------------------------------------------------------------------
//       TreePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

Rational TreePureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  PureBehaviorProfile behav(m_nfg);
  for (const auto &player : m_nfg->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      try {
        behav.SetAction(infoset->GetAction(m_profile.at(player)->m_behav[infoset.get()]));
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
