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
//                     class GameTreeActionRep
//========================================================================

bool GameTreeActionRep::Precedes(const GameNode &n) const
{
  GameNode node = n;

  while (node != node->GetGame()->GetRoot()) {
    if (node->GetPriorAction() == GameAction(const_cast<GameTreeActionRep *>(this))) {
      return true;
    }
    else {
      node = node->GetParent();
    }
  }
  return false;
}

void GameTreeActionRep::DeleteAction()
{
  if (m_infoset->NumActions() == 1) {
    throw UndefinedException();
  }

  m_infoset->GetGame()->IncrementVersion();
  size_t where;
  for (where = 1; where <= m_infoset->m_actions.size() && m_infoset->m_actions[where] != this;
       where++)
    ;

  m_infoset->RemoveAction(where);
  for (auto member : m_infoset->m_members) {
    member->m_children[where]->DeleteTree();
    member->m_children[where]->Invalidate();
    erase_atindex(member->m_children, where);
  }
  if (m_infoset->IsChanceInfoset()) {
    m_infoset->m_efg->NormalizeChanceProbs(m_infoset);
  }
  m_infoset->m_efg->ClearComputedValues();
  m_infoset->m_efg->Canonicalize();
}

GameInfoset GameTreeActionRep::GetInfoset() const { return m_infoset; }

//========================================================================
//                       class GameTreeInfosetRep
//========================================================================

GameTreeInfosetRep::GameTreeInfosetRep(GameTreeRep *p_efg, int p_number, GamePlayerRep *p_player,
                                       int p_actions)
  : m_efg(p_efg), m_number(p_number), m_player(p_player), m_actions(p_actions)
{
  std::generate(m_actions.begin(), m_actions.end(),
                [this, i = 1]() mutable { return new GameTreeActionRep(i++, "", this); });
  if (p_player->IsChance()) {
    m_probs = Array<Number>(m_actions.size());
    std::fill(m_probs.begin(), m_probs.end(), Rational(1, m_actions.size()));
  }
  m_player->m_infosets.push_back(this);
}

GameTreeInfosetRep::~GameTreeInfosetRep()
{
  std::for_each(m_actions.begin(), m_actions.end(), [](GameActionRep *a) { a->Invalidate(); });
}

Game GameTreeInfosetRep::GetGame() const { return m_efg; }

Array<GameAction> GameTreeInfosetRep::GetActions() const
{
  Array<GameAction> ret(m_actions.size());
  std::copy(m_actions.cbegin(), m_actions.cend(), ret.begin());
  return ret;
}

void GameTreeInfosetRep::SetPlayer(GamePlayer p_player)
{
  if (p_player->GetGame() != m_efg) {
    throw MismatchException();
  }
  if (m_player->IsChance() || p_player->IsChance()) {
    throw UndefinedException();
  }
  if (m_player == p_player) {
    return;
  }

  m_efg->IncrementVersion();
  m_player->m_infosets.erase(
      std::find(m_player->m_infosets.begin(), m_player->m_infosets.end(), this));
  m_player = p_player;
  p_player->m_infosets.push_back(this);

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

bool GameTreeInfosetRep::Precedes(GameNode p_node) const
{
  auto *node = dynamic_cast<GameTreeNodeRep *>(p_node.operator->());
  while (node->m_parent) {
    if (node->m_infoset == this) {
      return true;
    }
    else {
      node = node->m_parent;
    }
  }
  return false;
}

GameAction GameTreeInfosetRep::InsertAction(GameAction p_action /* =0 */)
{
  if (p_action && p_action->GetInfoset() != this) {
    throw MismatchException();
  }

  m_efg->IncrementVersion();
  int where = m_actions.size() + 1;
  if (p_action) {
    for (where = 1; m_actions[where] != p_action; where++)
      ;
  }

  auto *action = new GameTreeActionRep(where, "", this);
  m_actions.insert(std::next(m_actions.cbegin(), where - 1), action);
  if (m_player->IsChance()) {
    m_probs.insert(std::next(m_probs.cbegin(), where - 1), Number());
  }
  RenumberActions();
  for (const auto &member : m_members) {
    member->m_children.insert(std::next(member->m_children.cbegin(), where - 1),
                              new GameTreeNodeRep(m_efg, member));
  }

  m_efg->m_numNodes += m_members.size();

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return action;
}

void GameTreeInfosetRep::RemoveAction(int which)
{
  m_efg->IncrementVersion();
  m_actions[which]->Invalidate();
  erase_atindex(m_actions, which);
  if (m_player->IsChance()) {
    erase_atindex(m_probs, which);
  }
  RenumberActions();
}

void GameTreeInfosetRep::RemoveMember(GameTreeNodeRep *p_node)
{
  m_efg->IncrementVersion();
  m_members.erase(std::find(m_members.begin(), m_members.end(), p_node));
  if (m_members.empty()) {
    m_player->m_infosets.erase(
        std::find(m_player->m_infosets.begin(), m_player->m_infosets.end(), this));
    int iset = 1;
    for (auto &infoset : m_player->m_infosets) {
      infoset->m_number = iset++;
    }
    Invalidate();
  }
}

void GameTreeInfosetRep::Reveal(GamePlayer p_player)
{
  m_efg->IncrementVersion();
  for (auto action : m_actions) {
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
            newiset = member->LeaveInfoset();
          }
          else {
            member->SetInfoset(newiset);
          }
        }
      }
    }
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

GameNode GameTreeInfosetRep::GetMember(int p_index) const { return m_members[p_index - 1]; }

Array<GameNode> GameTreeInfosetRep::GetMembers() const
{
  Array<GameNode> ret(m_members.size());
  std::transform(m_members.cbegin(), m_members.cend(), ret.begin(),
                 [](GameTreeNodeRep *n) -> GameNode { return n; });
  return ret;
}

GamePlayer GameTreeInfosetRep::GetPlayer() const { return m_player; }

bool GameTreeInfosetRep::IsChanceInfoset() const { return m_player->IsChance(); }

//========================================================================
//                         class GameTreeNodeRep
//========================================================================

GameTreeNodeRep::GameTreeNodeRep(GameTreeRep *e, GameTreeNodeRep *p) : m_efg(e), m_parent(p) {}

GameTreeNodeRep::~GameTreeNodeRep()
{
  std::for_each(m_children.begin(), m_children.end(), [](GameNodeRep *n) { n->Invalidate(); });
}

Game GameTreeNodeRep::GetGame() const { return m_efg; }

Array<GameNode> GameTreeNodeRep::GetChildren() const
{
  Array<GameNode> ret(m_children.size());
  std::transform(m_children.cbegin(), m_children.cend(), ret.begin(),
                 [](GameTreeNodeRep *n) -> GameNode { return n; });
  return ret;
}

GameNode GameTreeNodeRep::GetNextSibling() const
{
  if (!m_parent || m_parent->m_children.back() == this) {
    return nullptr;
  }
  return *std::next(std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this));
}

GameNode GameTreeNodeRep::GetPriorSibling() const
{
  if (!m_parent || m_parent->m_children.front() == this) {
    return nullptr;
  }
  return *std::prev(std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this));
}

GameAction GameTreeNodeRep::GetPriorAction() const
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

void GameTreeNodeRep::DeleteOutcome(GameOutcomeRep *outc)
{
  m_efg->IncrementVersion();
  if (outc == m_outcome) {
    m_outcome = nullptr;
  }
  for (auto child : m_children) {
    child->DeleteOutcome(outc);
  }
}

void GameTreeNodeRep::SetOutcome(const GameOutcome &p_outcome)
{
  m_efg->IncrementVersion();
  if (p_outcome != m_outcome) {
    m_outcome = p_outcome;
    m_efg->ClearComputedValues();
  }
}

bool GameTreeNodeRep::IsSuccessorOf(GameNode p_node) const
{
  auto *n = const_cast<GameTreeNodeRep *>(this);
  while (n && n != p_node) {
    n = n->m_parent;
  }
  return (n == p_node);
}

bool GameTreeNodeRep::IsSubgameRoot() const
{
  // First take care of a couple easy cases
  if (m_children.empty() || m_infoset->NumMembers() > 1) {
    return false;
  }
  if (!m_parent) {
    return true;
  }

  // A node is a subgame root if and only if in every information set,
  // either all members succeed the node in the tree,
  // or all members do not succeed the node in the tree.
  for (auto player : m_efg->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      const bool precedes =
          infoset->GetMember(1)->IsSuccessorOf(const_cast<GameTreeNodeRep *>(this));
      for (size_t mem = 2; mem <= infoset->NumMembers(); mem++) {
        if (infoset->GetMember(mem)->IsSuccessorOf(const_cast<GameTreeNodeRep *>(this)) !=
            precedes) {
          return false;
        }
      }
    }
  }

  return true;
}

void GameTreeNodeRep::DeleteParent()
{
  if (!m_parent) {
    return;
  }
  m_efg->IncrementVersion();
  GameTreeNodeRep *oldParent = m_parent;

  oldParent->m_children.erase(
      std::find(oldParent->m_children.begin(), oldParent->m_children.end(), this));
  oldParent->DeleteTree();
  m_parent = oldParent->m_parent;
  if (m_parent) {
    std::replace(m_parent->m_children.begin(), m_parent->m_children.end(), oldParent, this);
  }
  else {
    m_efg->m_root = this;
  }

  oldParent->Invalidate();
  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

void GameTreeNodeRep::DeleteTree()
{
  m_efg->IncrementVersion();
  while (!m_children.empty()) {
    m_children.front()->DeleteTree();
    m_children.front()->Invalidate();
    erase_atindex(m_children, 1);
  }

  m_efg->m_numNodes--;

  if (m_infoset) {
    m_infoset->RemoveMember(this);
    m_infoset = nullptr;
  }

  m_outcome = nullptr;
  m_label = "";

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

void GameTreeNodeRep::CopySubtree(GameTreeNodeRep *src, GameTreeNodeRep *stop)
{
  m_efg->IncrementVersion();
  if (src == stop) {
    m_outcome = src->m_outcome;
    return;
  }

  if (src->m_children.size()) {
    AppendMove(src->m_infoset);
    for (auto dest_child = m_children.begin(), src_child = src->m_children.begin();
         src_child != src->m_children.end(); src_child++, dest_child++) {
      (*dest_child)->CopySubtree(*src_child, stop);
    }
  }

  m_label = src->m_label;
  m_outcome = src->m_outcome;
}

void GameTreeNodeRep::CopyTree(GameNode p_src)
{
  if (p_src->GetGame() != m_efg) {
    throw MismatchException();
  }
  if (p_src == this || !m_children.empty()) {
    return;
  }

  m_efg->IncrementVersion();
  auto *src = dynamic_cast<GameTreeNodeRep *>(p_src.operator->());

  if (!src->m_children.empty()) {
    AppendMove(src->m_infoset);
    for (auto dest_child = m_children.begin(), src_child = src->m_children.begin();
         src_child != src->m_children.end(); src_child++, dest_child++) {
      (*dest_child)->CopySubtree(*src_child, this);
    }
    m_efg->ClearComputedValues();
    m_efg->Canonicalize();
  }
}

void GameTreeNodeRep::MoveTree(GameNode p_src)
{
  if (p_src->GetGame() != m_efg) {
    throw MismatchException();
  }
  if (p_src == this || !m_children.empty() || IsSuccessorOf(p_src)) {
    return;
  }
  m_efg->IncrementVersion();
  auto *src = dynamic_cast<GameTreeNodeRep *>(p_src.operator->());
  std::iter_swap(
      std::find(src->m_parent->m_children.begin(), src->m_parent->m_children.end(), src),
      std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this));
  std::swap(src->m_parent, m_parent);
  m_label = "";
  m_outcome = nullptr;

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

Game GameTreeNodeRep::CopySubgame() const
{
  std::ostringstream os;
  m_efg->WriteEfgFile(os, const_cast<GameTreeNodeRep *>(this));
  std::istringstream is(os.str());
  return ReadGame(is);
}

void GameTreeNodeRep::SetInfoset(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) {
    throw MismatchException();
  }
  if (!m_infoset || m_infoset == p_infoset) {
    return;
  }
  if (p_infoset->NumActions() != m_children.size()) {
    throw DimensionException();
  }
  m_efg->IncrementVersion();
  m_infoset->RemoveMember(this);
  dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->())->AddMember(this);
  m_infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

GameInfoset GameTreeNodeRep::LeaveInfoset()
{
  if (!m_infoset) {
    return nullptr;
  }

  m_efg->IncrementVersion();
  GameTreeInfosetRep *oldInfoset = m_infoset;
  if (oldInfoset->m_members.size() == 1) {
    return oldInfoset;
  }

  GamePlayerRep *player = oldInfoset->m_player;
  oldInfoset->RemoveMember(this);
  m_infoset =
      new GameTreeInfosetRep(m_efg, player->m_infosets.size() + 1, player, m_children.size());
  m_infoset->AddMember(this);
  for (auto old_act = oldInfoset->m_actions.begin(), new_act = m_infoset->m_actions.begin();
       old_act != oldInfoset->m_actions.end(); ++old_act, ++new_act) {
    (*new_act)->SetLabel((*old_act)->GetLabel());
  }
  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return m_infoset;
}

GameInfoset GameTreeNodeRep::AppendMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0 || !m_children.empty()) {
    throw UndefinedException();
  }
  if (p_player->GetGame() != m_efg) {
    throw MismatchException();
  }

  m_efg->IncrementVersion();
  return AppendMove(
      new GameTreeInfosetRep(m_efg, p_player->m_infosets.size() + 1, p_player, p_actions));
}

GameInfoset GameTreeNodeRep::AppendMove(GameInfoset p_infoset)
{
  if (!m_children.empty()) {
    throw UndefinedException();
  }
  if (p_infoset->GetGame() != m_efg) {
    throw MismatchException();
  }

  m_efg->IncrementVersion();
  m_infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());
  m_infoset->AddMember(this);
  std::for_each(m_infoset->m_actions.begin(), m_infoset->m_actions.end(),
                [this](const GameActionRep *) {
                  m_children.push_back(new GameTreeNodeRep(m_efg, this));
                  m_efg->m_numNodes++;
                });
  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return m_infoset;
}

GameInfoset GameTreeNodeRep::InsertMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0) {
    throw UndefinedException();
  }
  if (p_player->GetGame() != m_efg) {
    throw MismatchException();
  }

  m_efg->IncrementVersion();
  return InsertMove(
      new GameTreeInfosetRep(m_efg, p_player->m_infosets.size() + 1, p_player, p_actions));
}

GameInfoset GameTreeNodeRep::InsertMove(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) {
    throw MismatchException();
  }

  m_efg->IncrementVersion();
  auto *newNode = new GameTreeNodeRep(m_efg, m_parent);
  newNode->m_infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());
  dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->())->AddMember(newNode);

  if (m_parent) {
    std::replace(m_parent->m_children.begin(), m_parent->m_children.end(), this, newNode);
  }
  else {
    m_efg->m_root = newNode;
  }

  m_parent = newNode;
  newNode->m_children.push_back(this);
  std::for_each(std::next(newNode->m_infoset->m_actions.begin()),
                newNode->m_infoset->m_actions.end(), [this, newNode](const GameActionRep *) {
                  newNode->m_children.push_back(new GameTreeNodeRep(m_efg, newNode));
                });

  // Total nodes added = 1 (newNode) + (NumActions - 1) (new children of newNode) = NumActions
  m_efg->m_numNodes += newNode->m_infoset->m_actions.size();

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return p_infoset;
}

//========================================================================
//                           class GameTreeRep
//========================================================================

//------------------------------------------------------------------------
//                        GameTreeRep: Lifecycle
//------------------------------------------------------------------------

GameTreeRep::GameTreeRep()
  : m_root(new GameTreeNodeRep(this, nullptr)), m_chance(new GamePlayerRep(this, 0))
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

Rational SubtreeSum(const GameNode &p_node)
{
  Rational sum(0);

  if (p_node->NumChildren() > 0) {
    sum = SubtreeSum(p_node->GetChild(1));
    for (size_t i = 2; i <= p_node->NumChildren(); i++) {
      if (SubtreeSum(p_node->GetChild(i)) != sum) {
        throw NotZeroSumException();
      }
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

bool GameTreeRep::IsPerfectRecall(GameInfoset &s1, GameInfoset &s2) const
{
  for (auto player : m_players) {
    for (size_t i = 1; i <= player->NumInfosets(); i++) {
      GameTreeInfosetRep *iset1 = player->m_infosets[i - 1];
      for (size_t j = 1; j <= player->NumInfosets(); j++) {
        GameTreeInfosetRep *iset2 = player->m_infosets[j - 1];

        bool precedes = false;
        size_t action = 0;

        for (size_t m = 1; m <= iset2->NumMembers(); m++) {
          size_t n;
          for (n = 1; n <= iset1->NumMembers(); n++) {
            if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)) &&
                iset1->GetMember(n) != iset2->GetMember(m)) {
              precedes = true;
              for (size_t act = 1; act <= iset1->NumActions(); act++) {
                if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)->GetChild(act))) {
                  if (action != 0 && action != act) {
                    s1 = iset1;
                    s2 = iset2;
                    return false;
                  }
                  action = act;
                }
              }
              break;
            }
          }

          if (i == j && precedes) {
            s1 = iset1;
            s2 = iset2;
            return false;
          }

          if (n > iset1->NumMembers() && precedes) {
            s1 = iset1;
            s2 = iset2;
            return false;
          }
        }
      }
    }
  }

  return true;
}

//------------------------------------------------------------------------
//               GameTreeRep: Managing the representation
//------------------------------------------------------------------------

void GameTreeRep::NumberNodes(GameTreeNodeRep *n, int &index)
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
    GamePlayerRep *player = (pl) ? m_players[pl] : m_chance;

    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (auto &infoset : player->m_infosets) {
      for (size_t i = 1; i < infoset->m_members.size(); i++) {
        for (size_t j = 1; j < infoset->m_members.size() - i; j++) {
          if (infoset->m_members[j]->m_number < infoset->m_members[j - 1]->m_number) {
            GameTreeNodeRep *tmp = infoset->m_members[j - 1];
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
          GameTreeInfosetRep *tmp = player->m_infosets[j - 1];
          player->m_infosets[j - 1] = player->m_infosets[j];
          player->m_infosets[j] = tmp;
        }
      }
    }

    // Reassign information set IDs
    std::for_each(player->m_infosets.begin(), player->m_infosets.end(),
                  [iset = 1](GameTreeInfosetRep *s) mutable { s->m_number = iset++; });
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
  m_computedValues = false;
}

void GameTreeRep::BuildComputedValues()
{
  if (m_computedValues) {
    return;
  }
  Canonicalize();
  for (const auto &player : m_players) {
    player->MakeReducedStrats(m_root, nullptr);
  }
  m_computedValues = true;
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
      f << FormatList(n->GetInfoset()->GetActions(), [n](const GameAction &a) {
        return QuoteString(a->GetLabel()) + " " + std::string(a->GetInfoset()->GetActionProb(a));
      });
    }
    else {
      f << FormatList(n->GetInfoset()->GetActions(),
                      [n](const GameAction &a) { return QuoteString(a->GetLabel()); });
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
  // FIXME: Building computed values is logically const.
  const_cast<GameTreeRep *>(this)->BuildComputedValues();
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

Array<GameInfoset> GameTreeRep::GetInfosets() const
{
  Array<GameInfoset> infosets;
  for (auto player : m_players) {
    for (auto infoset : player->m_infosets) {
      infosets.push_back(infoset);
    }
  }
  return infosets;
}

Array<int> GameTreeRep::NumInfosets() const
{
  Array<int> foo;
  for (const auto &player : m_players) {
    foo.push_back(player->NumInfosets());
  }
  return foo;
}

//------------------------------------------------------------------------
//                        GameTreeRep: Outcomes
//------------------------------------------------------------------------

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
  if (p_infoset->NumActions() != p_probs.size()) {
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
  std::copy(p_probs.begin(), p_probs.end(),
            dynamic_cast<GameTreeInfosetRep &>(*p_infoset).m_probs.begin());
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
  auto &probs = dynamic_cast<GameTreeInfosetRep &>(*p_infoset).m_probs;
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
    for (size_t iset = 1; iset <= player->NumInfosets(); iset++) {
      const int act = m_profile.at(player)->m_behav[iset];
      if (act) {
        behav.SetAction(player->GetInfoset(iset)->GetAction(act));
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
