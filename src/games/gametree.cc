//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include <numeric>
#include <optional>

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

template <class T> MixedStrategyProfileRep<T> *TreeMixedStrategyProfileRep<T>::Copy() const
{
  return new TreeMixedStrategyProfileRep(*this);
}

template <class T> void TreeMixedStrategyProfileRep<T>::MakeBehavior() const
{
  if (mixed_behav_profile_sptr.get() == nullptr) {
    mixed_behav_profile_sptr =
        std::make_shared<MixedBehaviorProfile<T>>(MixedStrategyProfile<T>(Copy()));
  }
}

template <class T> void TreeMixedStrategyProfileRep<T>::InvalidateCache() const
{
  mixed_behav_profile_sptr = nullptr;
}

template <class T> T TreeMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  MakeBehavior();
  return mixed_behav_profile_sptr->GetPayoff(pl);
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
    if (node->GetPriorAction() == GameAction(const_cast<GameActionRep *>(this))) {
      return true;
    }
    else {
      node = node->GetParent();
    }
  }
  return false;
}

void GameTreeRep::DeleteAction(GameAction p_action)
{
  GameActionRep *action = p_action;
  auto *infoset = action->m_infoset;
  if (infoset->m_game != this) {
    throw MismatchException();
  }
  if (infoset->m_actions.size() == 1) {
    throw UndefinedException();
  }

  IncrementVersion();
  auto where = std::find(infoset->m_actions.begin(), infoset->m_actions.end(), action);
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
  Canonicalize();
}

GameInfoset GameActionRep::GetInfoset() const { return m_infoset; }

//========================================================================
//                       class GameInfosetRep
//========================================================================

GameInfosetRep::GameInfosetRep(GameRep *p_efg, int p_number, GamePlayerRep *p_player,
                               int p_actions)
  : m_game(p_efg), m_number(p_number), m_player(p_player), m_actions(p_actions)
{
  std::generate(m_actions.begin(), m_actions.end(),
                [this, i = 1]() mutable { return new GameActionRep(i++, "", this); });
  if (p_player->IsChance()) {
    m_probs = std::vector<Number>(m_actions.size());
    std::fill(m_probs.begin(), m_probs.end(), Rational(1, m_actions.size()));
  }
  m_player->m_infosets.push_back(this);
}

GameInfosetRep::~GameInfosetRep()
{
  std::for_each(m_actions.begin(), m_actions.end(), [](GameActionRep *a) { a->Invalidate(); });
}

void GameTreeRep::SetPlayer(GameInfoset p_infoset, GamePlayer p_player)
{
  if (p_infoset->GetGame() != this || p_player->GetGame() != this) {
    throw MismatchException();
  }
  if (p_infoset->GetPlayer()->IsChance() || p_player->IsChance()) {
    throw UndefinedException();
  }
  if (p_infoset->GetPlayer() == p_player) {
    return;
  }

  GamePlayerRep *oldPlayer = p_infoset->GetPlayer();
  IncrementVersion();
  oldPlayer->m_infosets.erase(
      std::find(oldPlayer->m_infosets.begin(), oldPlayer->m_infosets.end(), p_infoset));
  p_infoset->m_player = p_player;
  p_player->m_infosets.push_back(p_infoset);

  ClearComputedValues();
  Canonicalize();
}

bool GameInfosetRep::Precedes(GameNode p_node) const
{
  auto node = p_node;
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
  auto where = std::find(p_infoset->m_actions.begin(), p_infoset->m_actions.end(), p_action);
  auto offset = where - p_infoset->m_actions.begin();

  auto *action = new GameActionRep(offset + 1, "", p_infoset);
  p_infoset->m_actions.insert(where, action);
  if (p_infoset->m_player->IsChance()) {
    p_infoset->m_probs.insert(std::next(p_infoset->m_probs.cbegin(), offset), Number());
  }
  p_infoset->RenumberActions();
  for (const auto &member : p_infoset->m_members) {
    member->m_children.insert(std::next(member->m_children.cbegin(), offset),
                              new GameNodeRep(this, member));
  }

  m_numNodes += p_infoset->m_members.size();
  // m_numNonterminalNodes stays unchanged when an action is appended to an information set
  ClearComputedValues();
  Canonicalize();
  return action;
}

void GameTreeRep::RemoveMember(GameInfosetRep *p_infoset, GameNodeRep *p_node)
{
  IncrementVersion();
  p_infoset->m_members.erase(
      std::find(p_infoset->m_members.begin(), p_infoset->m_members.end(), p_node));
  if (p_infoset->m_members.empty()) {
    p_infoset->m_player->m_infosets.erase(std::find(p_infoset->m_player->m_infosets.begin(),
                                                    p_infoset->m_player->m_infosets.end(),
                                                    p_infoset));
    int iset = 1;
    for (auto &infoset : p_infoset->m_player->m_infosets) {
      infoset->m_number = iset++;
    }
    p_infoset->Invalidate();
  }
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
  Canonicalize();
}

//========================================================================
//                         class GameNodeRep
//========================================================================

GameNodeRep::GameNodeRep(GameRep *e, GameNodeRep *p) : m_game(e), m_parent(p) {}

GameNodeRep::~GameNodeRep()
{
  std::for_each(m_children.begin(), m_children.end(), [](GameNodeRep *n) { n->Invalidate(); });
}

GameNode GameNodeRep::GetNextSibling() const
{
  if (!m_parent || m_parent->m_children.back() == this) {
    return nullptr;
  }
  return *std::next(std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this));
}

GameNode GameNodeRep::GetPriorSibling() const
{
  if (!m_parent || m_parent->m_children.front() == this) {
    return nullptr;
  }
  return *std::prev(std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this));
}

GameAction GameNodeRep::GetPriorAction() const
{
  if (!m_parent) {
    return nullptr;
  }
  for (const auto &action : m_parent->m_infoset->m_actions) {
    if (m_parent->GetChild(action) == GameNode(this)) {
      return action;
    }
  }
  return nullptr;
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

void GameTreeRep::SetOutcome(GameNode p_node, const GameOutcome &p_outcome)
{
  IncrementVersion();
  if (p_node->GetGame() != this) {
    throw MismatchException();
  }
  if (p_outcome && p_outcome->GetGame() != this) {
    throw MismatchException();
  }
  if (p_outcome != p_node->m_outcome) {
    p_node->m_outcome = p_outcome;
    ClearComputedValues();
  }
}

bool GameNodeRep::IsSuccessorOf(GameNode p_node) const
{
  auto *n = const_cast<GameNodeRep *>(this);
  while (n && n != p_node) {
    n = n->m_parent;
  }
  return (n == p_node);
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
      const bool precedes =
          infoset->m_members.front()->IsSuccessorOf(const_cast<GameNodeRep *>(this));
      if (std::any_of(std::next(infoset->m_members.begin()), infoset->m_members.end(),
                      [this, precedes](GameNodeRep *m) {
                        return m->IsSuccessorOf(const_cast<GameNodeRep *>(this)) != precedes;
                      })) {
        return false;
      }
    }
  }

  return true;
}

void GameTreeRep::DeleteParent(GameNode p_node)
{
  if (p_node->GetGame() != this) {
    throw MismatchException();
  }
  auto *node = dynamic_cast<GameNodeRep *>(p_node.operator->());
  if (!node->m_parent) {
    return;
  }
  IncrementVersion();
  auto *oldParent = node->m_parent;

  oldParent->m_children.erase(
      std::find(oldParent->m_children.begin(), oldParent->m_children.end(), node));
  DeleteTree(oldParent);
  node->m_parent = oldParent->m_parent;
  m_numNodes--;
  if (node->m_parent) {
    std::replace(node->m_parent->m_children.begin(), node->m_parent->m_children.end(), oldParent,
                 node);
  }
  else {
    m_root = node;
  }

  oldParent->Invalidate();
  ClearComputedValues();
  Canonicalize();
}

void GameTreeRep::DeleteTree(GameNode p_node)
{
  if (p_node->GetGame() != this) {
    throw MismatchException();
  }
  GameNodeRep *node = p_node;
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
  Canonicalize();
}

void GameTreeRep::CopySubtree(GameNodeRep *dest, GameNodeRep *src, GameNodeRep *stop)
{
  IncrementVersion();
  if (src == stop) {
    dest->m_outcome = src->m_outcome;
    return;
  }

  if (!src->m_children.empty()) {
    AppendMove(dest, src->m_infoset);
    for (auto dest_child = dest->m_children.begin(), src_child = src->m_children.begin();
         src_child != src->m_children.end(); src_child++, dest_child++) {
      CopySubtree(*dest_child, *src_child, stop);
    }
  }

  dest->m_label = src->m_label;
  dest->m_outcome = src->m_outcome;
}

void GameTreeRep::CopyTree(GameNode p_dest, GameNode p_src)
{
  if (p_dest->GetGame() != this || p_src->GetGame() != this) {
    throw MismatchException();
  }
  GameNodeRep *dest = p_dest;
  GameNodeRep *src = p_src;
  if (dest == src || !dest->m_children.empty()) {
    return;
  }

  IncrementVersion();
  if (!src->m_children.empty()) {
    AppendMove(dest, src->m_infoset);
    for (auto dest_child = dest->m_children.begin(), src_child = src->m_children.begin();
         src_child != src->m_children.end(); src_child++, dest_child++) {
      CopySubtree(*dest_child, *src_child, dest);
    }
    ClearComputedValues();
    Canonicalize();
  }
}

void GameTreeRep::MoveTree(GameNode p_dest, GameNode p_src)
{
  if (p_dest->GetGame() != this || p_src->GetGame() != this) {
    throw MismatchException();
  }
  GameNodeRep *dest = p_dest;
  GameNodeRep *src = p_src;
  if (src == dest || !dest->m_children.empty() || p_dest->IsSuccessorOf(p_src)) {
    return;
  }
  IncrementVersion();
  std::iter_swap(
      std::find(src->m_parent->m_children.begin(), src->m_parent->m_children.end(), src),
      std::find(dest->m_parent->m_children.begin(), dest->m_parent->m_children.end(), dest));
  std::swap(src->m_parent, dest->m_parent);
  dest->m_label = "";
  dest->m_outcome = nullptr;

  ClearComputedValues();
  Canonicalize();
}

Game GameTreeRep::CopySubgame(GameNode p_root) const
{
  if (p_root->GetGame() != const_cast<GameTreeRep *>(this)) {
    throw MismatchException();
  }
  std::ostringstream os;
  WriteEfgFile(os, p_root);
  std::istringstream is(os.str());
  return ReadGame(is);
}

void GameTreeRep::SetInfoset(GameNode p_node, GameInfoset p_infoset)
{
  if (p_node->GetGame() != this || p_infoset->GetGame() != this) {
    throw MismatchException();
  }
  GameNodeRep *node = p_node;
  if (!node->m_infoset || node->m_infoset == p_infoset) {
    return;
  }
  if (p_infoset->m_actions.size() != node->m_children.size()) {
    throw DimensionException();
  }
  IncrementVersion();
  RemoveMember(node->m_infoset, node);
  p_infoset->m_members.push_back(node);
  node->m_infoset = p_infoset;

  ClearComputedValues();
  Canonicalize();
}

GameInfoset GameTreeRep::LeaveInfoset(GameNode p_node)
{
  GameNodeRep *node = p_node;
  if (node->m_game != this) {
    throw MismatchException();
  }
  if (!node->m_infoset) {
    return nullptr;
  }

  IncrementVersion();
  auto *oldInfoset = node->m_infoset;
  if (oldInfoset->m_members.size() == 1) {
    return oldInfoset;
  }

  GamePlayerRep *player = oldInfoset->m_player;
  RemoveMember(oldInfoset, node);
  node->m_infoset =
      new GameInfosetRep(this, player->m_infosets.size() + 1, player, node->m_children.size());
  node->m_infoset->m_members.push_back(node);
  for (auto old_act = oldInfoset->m_actions.begin(), new_act = node->m_infoset->m_actions.begin();
       old_act != oldInfoset->m_actions.end(); ++old_act, ++new_act) {
    (*new_act)->SetLabel((*old_act)->GetLabel());
  }
  ClearComputedValues();
  Canonicalize();
  return node->m_infoset;
}

GameInfoset GameTreeRep::AppendMove(GameNode p_node, GamePlayer p_player, int p_actions)
{
  GameNodeRep *node = p_node;
  if (p_actions <= 0 || !node->m_children.empty()) {
    throw UndefinedException();
  }
  if (p_node->GetGame() != this || p_player->GetGame() != this) {
    throw MismatchException();
  }

  IncrementVersion();
  return AppendMove(
      p_node, new GameInfosetRep(this, p_player->m_infosets.size() + 1, p_player, p_actions));
}

GameInfoset GameTreeRep::AppendMove(GameNode p_node, GameInfoset p_infoset)
{
  GameNodeRep *node = p_node;
  if (!node->m_children.empty()) {
    throw UndefinedException();
  }
  if (p_node->GetGame() != this || p_infoset->GetGame() != this) {
    throw MismatchException();
  }

  IncrementVersion();
  node->m_infoset = p_infoset;
  node->m_infoset->m_members.push_back(node);
  std::for_each(node->m_infoset->m_actions.begin(), node->m_infoset->m_actions.end(),
                [this, node](const GameActionRep *) {
                  node->m_children.push_back(new GameNodeRep(this, node));
                  m_numNodes++;
                });
  m_numNonterminalNodes++;
  ClearComputedValues();
  Canonicalize();
  return node->m_infoset;
}

GameInfoset GameTreeRep::InsertMove(GameNode p_node, GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0) {
    throw UndefinedException();
  }
  if (p_player->GetGame() != this) {
    throw MismatchException();
  }

  IncrementVersion();
  return InsertMove(
      p_node, new GameInfosetRep(this, p_player->m_infosets.size() + 1, p_player, p_actions));
}

GameInfoset GameTreeRep::InsertMove(GameNode p_node, GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != this) {
    throw MismatchException();
  }

  IncrementVersion();
  GameNodeRep *node = p_node;
  auto *newNode = new GameNodeRep(this, node->m_parent);
  newNode->m_infoset = p_infoset;
  p_infoset->m_members.push_back(newNode);

  if (node->m_parent) {
    std::replace(node->m_parent->m_children.begin(), node->m_parent->m_children.end(), node,
                 newNode);
  }
  else {
    m_root = newNode;
  }

  node->m_parent = newNode;
  newNode->m_children.push_back(node);
  std::for_each(std::next(newNode->m_infoset->m_actions.begin()),
                newNode->m_infoset->m_actions.end(), [this, newNode](const GameActionRep *) {
                  newNode->m_children.push_back(new GameNodeRep(this, newNode));
                });

  // Total nodes added = 1 (newNode) + (NumActions - 1) (new children of newNode) = NumActions
  m_numNodes += newNode->m_infoset->m_actions.size();
  m_numNonterminalNodes++;
  ClearComputedValues();
  Canonicalize();
  return p_infoset;
}

//========================================================================
//                           class GameTreeRep
//========================================================================

//------------------------------------------------------------------------
//                        GameTreeRep: Lifecycle
//------------------------------------------------------------------------

GameTreeRep::GameTreeRep()
  : m_root(new GameNodeRep(this, nullptr)), m_chance(new GamePlayerRep(this, 0))
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

Game NewTree() { return new GameTreeRep(); }

//------------------------------------------------------------------------
//                 GameTreeRep: General data access
//------------------------------------------------------------------------

namespace {

class NotZeroSumException : public Exception {
public:
  ~NotZeroSumException() noexcept override = default;
  const char *what() const noexcept override { return "Game is not constant sum"; }
};

Rational SubtreeSum(GameNode p_node)
{
  Rational sum(0);

  if (!p_node->IsTerminal()) {
    auto children = p_node->GetChildren();
    sum = SubtreeSum(children.front());
    if (std::any_of(std::next(children.begin()), children.end(),
                    [sum](GameNode n) { return SubtreeSum(n) != sum; })) {
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

bool GameTreeRep::IsPerfectRecall() const
{
  using Assignment = std::pair<GameInfoset, GameAction>;
  // Histories track the last actions of players taken to reach a given node
  using History = std::map<GamePlayer, std::optional<Assignment>>;

  std::map<GameInfoset, std::optional<Assignment>> last_actions_for_infoset;
  std::map<GameNode, History> last_actions_for_node;

  History initial_history;
  for (const auto &player : GetPlayers()) {
    initial_history[player] = std::nullopt;
  }
  initial_history[GetChance()] = std::nullopt;
  last_actions_for_node[GetRoot()] = initial_history;

  for (const GameNode n : *this) {
    History &history = last_actions_for_node.at(n);

    if (n->IsTerminal()) {
      last_actions_for_node.erase(n);
      continue;
    }

    const GamePlayer player = n->GetPlayer();
    const GameInfoset infoset = n->GetInfoset();
    const std::optional<Assignment> &player_last_action = history.at(player);

    auto [it, inserted] = last_actions_for_infoset.try_emplace(infoset, player_last_action);

    if (!inserted) {
      if (it->second != player_last_action) {
        if (!player->IsChance()) {
          return false;
        }
      }
    }

    for (const GameAction action : infoset->GetActions()) {
      const GameNode child = n->GetChild(action);
      History child_history = history;
      child_history.at(player) = std::make_pair(infoset, action);
      last_actions_for_node[child] = child_history;
    }

    last_actions_for_node.erase(n);
  }

  return true;
}

//------------------------------------------------------------------------
//               GameTreeRep: Managing the representation
//------------------------------------------------------------------------

void GameTreeRep::NumberNodes(GameNodeRep *n, int &index)
{
  n->m_number = index++;
  for (auto &child : n->m_children) {
    NumberNodes(child, index);
  }
}

void GameTreeRep::Canonicalize()
{
  if (!m_doCanon) {
    return;
  }
  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);

  for (size_t pl = 0; pl <= m_players.size(); pl++) {
    GamePlayerRep *player = (pl) ? m_players[pl - 1] : m_chance;

    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (auto &infoset : player->m_infosets) {
      for (size_t i = 1; i < infoset->m_members.size(); i++) {
        for (size_t j = 1; j < infoset->m_members.size() - i; j++) {
          if (infoset->m_members[j]->m_number < infoset->m_members[j - 1]->m_number) {
            GameNodeRep *tmp = infoset->m_members[j - 1];
            infoset->m_members[j - 1] = infoset->m_members[j];
            infoset->m_members[j] = tmp;
          }
        }
      }
    }

    // Sort information sets by the smallest ID among their members
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (size_t i = 1; i < player->m_infosets.size(); i++) {
      for (size_t j = 1; j < player->m_infosets.size() - i; j++) {
        const int a = ((player->m_infosets[j]->m_members.size())
                           ? player->m_infosets[j]->m_members[0]->m_number
                           : 0);
        const int b = ((player->m_infosets[j - 1]->m_members.size())
                           ? player->m_infosets[j - 1]->m_members[0]->m_number
                           : 0);

        if (a < b || b == 0) {
          auto *tmp = player->m_infosets[j - 1];
          player->m_infosets[j - 1] = player->m_infosets[j];
          player->m_infosets[j] = tmp;
        }
      }
    }

    // Reassign information set IDs
    std::for_each(player->m_infosets.begin(), player->m_infosets.end(),
                  [iset = 1](GameInfosetRep *s) mutable { s->m_number = iset++; });
  }
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
  m_computedValues = false;
}

void GameTreeRep::BuildComputedValues() const
{
  if (m_computedValues) {
    return;
  }
  const_cast<GameTreeRep *>(this)->Canonicalize();
  for (const auto &player : m_players) {
    player->MakeReducedStrats(m_root, nullptr);
  }
  m_computedValues = true;
}

void GameTreeRep::BuildConsistentPlays()
{
  m_nodePlays.clear();
  BuildConsistentPlaysRecursiveImpl(m_root);
}

std::vector<GameNodeRep *> GameTreeRep::BuildConsistentPlaysRecursiveImpl(GameNodeRep *node)
{
  std::vector<GameNodeRep *> consistent_plays;
  if (node->IsTerminal()) {
    consistent_plays = std::vector<GameNodeRep *>{node};
  }
  else {
    for (GameNodeRep *child : node->GetChildren()) {
      auto child_consistent_plays = BuildConsistentPlaysRecursiveImpl(child);
      consistent_plays.insert(consistent_plays.end(), child_consistent_plays.begin(),
                              child_consistent_plays.end());
    }
  }
  m_nodePlays[node] = consistent_plays;
  return consistent_plays;
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
  auto player = new GamePlayerRep(this, m_players.size() + 1);
  m_players.push_back(player);
  for (auto &outcome : m_outcomes) {
    outcome->m_payoffs[player] = Number();
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
  throw IndexException();
}

std::vector<GameInfoset> GameTreeRep::GetInfosets() const
{
  std::vector<GameInfoset> infosets;
  for (const auto &player : m_players) {
    std::copy(player->m_infosets.begin(), player->m_infosets.end(), std::back_inserter(infosets));
  }
  return infosets;
}

//------------------------------------------------------------------------
//                        GameTreeRep: Outcomes
//------------------------------------------------------------------------

std::vector<GameNode> GameTreeRep::GetPlays(GameNode node) const
{
  const_cast<GameTreeRep *>(this)->BuildConsistentPlays();

  const std::vector<GameNodeRep *> &consistent_plays = m_nodePlays.at(node);
  std::vector<GameNode> consistent_plays_copy;
  consistent_plays_copy.reserve(consistent_plays.size());

  std::transform(consistent_plays.cbegin(), consistent_plays.cend(),
                 std::back_inserter(consistent_plays_copy),
                 [](GameNodeRep *rep_ptr) -> GameNode { return {rep_ptr}; });

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
  m_root->DeleteOutcome(p_outcome);
  m_outcomes.erase(std::find(m_outcomes.begin(), m_outcomes.end(), p_outcome));
  p_outcome->Invalidate();
  std::for_each(m_outcomes.begin(), m_outcomes.end(),
                [outc = 1](GameOutcomeRep *c) mutable { c->m_number = outc++; });
  ClearComputedValues();
}

//------------------------------------------------------------------------
//                       GameTreeRep: Modification
//------------------------------------------------------------------------

Game GameTreeRep::SetChanceProbs(const GameInfoset &p_infoset, const Array<Number> &p_probs)
{
  if (p_infoset->GetGame() != this) {
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
  return this;
}

Game GameTreeRep::NormalizeChanceProbs(const GameInfoset &p_infoset)
{
  if (p_infoset->GetGame() != this) {
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
  return this;
}

//------------------------------------------------------------------------
//                     GameTreeRep: Factory functions
//------------------------------------------------------------------------

MixedStrategyProfile<double> GameTreeRep::NewMixedStrategyProfile(double) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  return StrategySupportProfile(const_cast<GameTreeRep *>(this)).NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTreeRep::NewMixedStrategyProfile(const Rational &) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  return StrategySupportProfile(const_cast<GameTreeRep *>(this))
      .NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double>
GameTreeRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  return MixedStrategyProfile<double>(new TreeMixedStrategyProfileRep<double>(spt));
}

MixedStrategyProfile<Rational>
GameTreeRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }
  return MixedStrategyProfile<Rational>(new TreeMixedStrategyProfileRep<Rational>(spt));
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
  return PureStrategyProfile(
      std::make_shared<TreePureStrategyProfileRep>(const_cast<GameTreeRep *>(this)));
}

//------------------------------------------------------------------------
//       TreePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

Rational TreePureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  PureBehaviorProfile behav(m_nfg);
  for (const auto &player : m_nfg->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      if (const int act = m_profile.at(player)->m_behav[infoset]) {
        behav.SetAction(infoset->GetAction(act));
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
