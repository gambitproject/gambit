//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit.h"
#include "gametree.h"

namespace Gambit {

//========================================================================
//                   class TreeMixedStrategyProfileRep
//========================================================================

//========================================================================
//                   TreeMixedStrategyProfileRep<T>
//========================================================================

template <class T>
TreeMixedStrategyProfileRep<T>::TreeMixedStrategyProfileRep(const MixedBehaviorProfile<T> &p_profile)
  : MixedStrategyProfileRep<T>(p_profile.GetGame())
{ }

template <class T>
MixedStrategyProfileRep<T> *TreeMixedStrategyProfileRep<T>::Copy() const
{
  return new TreeMixedStrategyProfileRep(*this);
}

template <class T> T TreeMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  MixedStrategyProfile<T> profile(Copy());
  return MixedBehaviorProfile<T>(profile).GetPayoff(pl);
}

template <class T> T
TreeMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl,
                                               const GameStrategy &strategy) const
{
  MixedStrategyProfile<T> foo(Copy());
  for (auto s : this->m_support.GetStrategies(this->m_support.GetGame()->GetPlayer(pl))) {
    foo[s] = static_cast<T>(0);
  }
  foo[strategy] = static_cast<T>(1);
  return foo.GetPayoff(pl);
}

template <class T> T
TreeMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl,
                                               const GameStrategy &strategy1,
                                               const GameStrategy &strategy2) const
{
  GamePlayerRep *player1 = strategy1->GetPlayer();
  GamePlayerRep *player2 = strategy2->GetPlayer();
  if (player1 == player2) return (T) 0;

  MixedStrategyProfile<T> foo(Copy());
  for (auto strategy : this->m_support.GetStrategies(player1)) {
    foo[strategy] = (T) 0;
  }
  foo[strategy1] = (T) 1;

  for (auto strategy : this->m_support.GetStrategies(player2)) {
    foo[strategy] = (T) 0;
  }
  foo[strategy2] = (T) 1;

  return foo.GetPayoff(pl);
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
  if (m_infoset->NumActions() == 1) throw UndefinedException();

  m_infoset->GetGame()->IncrementVersion();
  int where;
  for (where = 1;
       where <= m_infoset->m_actions.Length() && 
	 m_infoset->m_actions[where] != this;
       where++);

  m_infoset->RemoveAction(where);
  for (auto member : m_infoset->m_members) {
    member->children[where]->DeleteTree();
    member->children.Remove(where)->Invalidate();
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

GameTreeInfosetRep::GameTreeInfosetRep(GameTreeRep *p_efg, int p_number,
				       GamePlayerRep *p_player,
				       int p_actions)
  : m_efg(p_efg), m_number(p_number), m_player(p_player), 
    m_actions(p_actions), flag(0) 
{
  while (p_actions)   {
    m_actions[p_actions] = new GameTreeActionRep(p_actions, "", this);
    p_actions--; 
  }

  m_player->m_infosets.push_back(this);

  if (p_player->IsChance()) {
    m_probs = Array<Number>(m_actions.size());
    std::string prob = lexical_cast<std::string>(Rational(1, m_actions.Length()));
    for (int act = 1; act <= m_actions.Length(); act++) {
      m_probs[act] = prob;
    }
  }
}

GameTreeInfosetRep::~GameTreeInfosetRep()  
{
  std::for_each(m_actions.begin(), m_actions.end(),
                [](GameActionRep *a) { a->Invalidate(); });
}

Game GameTreeInfosetRep::GetGame() const { return m_efg; }

Array<GameAction> GameTreeInfosetRep::GetActions() const
{
  Array<GameAction> ret(m_actions.size());
  std::transform(m_actions.cbegin(), m_actions.end(),
                 ret.begin(), [](GameActionRep *a) -> GameAction { return a; });
  return ret;
}

void GameTreeInfosetRep::SetPlayer(GamePlayer p_player)
{
  if (p_player->GetGame() != m_efg) throw MismatchException();
  if (m_player->IsChance() || p_player->IsChance()) throw UndefinedException();
  if (m_player == p_player) return;

  m_efg->IncrementVersion();
  m_player->m_infosets.Remove(m_player->m_infosets.Find(this));
  m_player = p_player;
  p_player->m_infosets.push_back(this);

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

bool GameTreeInfosetRep::Precedes(GameNode p_node) const
{
  auto *node = dynamic_cast<GameTreeNodeRep *>(p_node.operator->());
  while (node->m_parent) {
    if (node->infoset == this) {
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
  if (p_action && p_action->GetInfoset() != this) throw MismatchException();

  m_efg->IncrementVersion();
  int where = m_actions.Length() + 1;
  if (p_action) {
    for (where = 1; m_actions[where] != p_action; where++); 
  }

  auto *action = new GameTreeActionRep(where, "", this);
  m_actions.Insert(action, where);
  if (m_player->IsChance()) {
    m_probs.Insert(Number("0"), where);
  }

  for (int act = 1; act <= m_actions.Length(); act++) {
    m_actions[act]->m_number = act;
  }

  for (int i = 1; i <= m_members.Length(); i++) {
    m_members[i]->children.Insert(new GameTreeNodeRep(m_efg, m_members[i]), 
				  where);
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return action;
}


void GameTreeInfosetRep::RemoveAction(int which)
{
  m_efg->IncrementVersion();
  m_actions.Remove(which)->Invalidate();
  if (m_player->IsChance()) {
    m_probs.Remove(which);
  }
  for (; which <= m_actions.Length(); which++) {
    m_actions[which]->m_number = which;
  }
}

void GameTreeInfosetRep::RemoveMember(GameTreeNodeRep *p_node)
{
  m_efg->IncrementVersion();
  m_members.Remove(m_members.Find(p_node));
  if (m_members.Length() == 0) {
    m_player->m_infosets.Remove(m_player->m_infosets.Find(this));
    for (int i = 1; i <= m_player->m_infosets.Length(); i++) {
      m_player->m_infosets[i]->m_number = i;
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
      for (int m = 1; m <= members.Length(); m++) {
        if (action->Precedes(members[m])) {
          if (!newiset) {
            newiset = members[m]->LeaveInfoset();
          }
          else {
            members[m]->SetInfoset(newiset);
          }
        }
      }
    }
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

GameNode GameTreeInfosetRep::GetMember(int p_index) const 
{
  return m_members[p_index];
}

Array<GameNode> GameTreeInfosetRep::GetMembers() const
{
  Array<GameNode> ret(m_members.size());
  std::transform(m_members.cbegin(), m_members.cend(),
                 ret.begin(), [](GameTreeNodeRep *n) -> GameNode { return n; });
  return ret;
}

GamePlayer GameTreeInfosetRep::GetPlayer() const
{
  return m_player;
}

bool GameTreeInfosetRep::IsChanceInfoset() const
{ return m_player->IsChance(); }


//========================================================================
//                         class GameTreeNodeRep
//========================================================================

GameTreeNodeRep::GameTreeNodeRep(GameTreeRep *e, GameTreeNodeRep *p)
  : number(0), m_efg(e), infoset(nullptr), m_parent(p), outcome(nullptr)
{ }

GameTreeNodeRep::~GameTreeNodeRep()
{
  std::for_each(children.begin(), children.end(),
                [](GameNodeRep *n) { n->Invalidate(); });
}

Game GameTreeNodeRep::GetGame() const { return m_efg; }

Array<GameNode> GameTreeNodeRep::GetChildren() const
{
  Array<GameNode> ret(children.size());
  std::transform(children.cbegin(), children.cend(),
                 ret.begin(), [](GameTreeNodeRep *n) -> GameNode { return n; });
  return ret;
}

GameNode GameTreeNodeRep::GetNextSibling() const
{
  if (!m_parent) {
    return nullptr;
  }
  if (m_parent->children.back() == this) {
    return nullptr;
  }
  else {
    return m_parent->children[m_parent->children.Find(const_cast<GameTreeNodeRep *>(this)) + 1];
  }
}

GameNode GameTreeNodeRep::GetPriorSibling() const
{ 
  if (!m_parent) {
    return nullptr;
  }
  if (m_parent->children.front() == this) {
    return nullptr;
  }
  else {
    return m_parent->children[m_parent->children.Find(const_cast<GameTreeNodeRep *>(this)) - 1];
  }
}

GameAction GameTreeNodeRep::GetPriorAction() const
{
  if (!m_parent) {
    return nullptr;
  }
  GameTreeInfosetRep *infoset = m_parent->infoset;
  for (int i = 1; i <= infoset->NumActions(); i++) {
    if (GameNode(const_cast<GameTreeNodeRep *>(this)) == GetParent()->GetChild(i)) {
      return infoset->GetAction(i);
    }
  }
  return nullptr;
}

void GameTreeNodeRep::DeleteOutcome(GameOutcomeRep *outc)
{
  m_efg->IncrementVersion();
  if (outc == outcome)   outcome = nullptr;
  for (auto child : children) {
    child->DeleteOutcome(outc);
  }
}

void GameTreeNodeRep::SetOutcome(const GameOutcome &p_outcome)
{
  m_efg->IncrementVersion();
  if (p_outcome != outcome) {
    outcome = p_outcome;
    m_efg->ClearComputedValues();
  }
}

bool GameTreeNodeRep::IsSuccessorOf(GameNode p_node) const
{
  auto *n = const_cast<GameTreeNodeRep *>(this);
  while (n && n != p_node) n = n->m_parent;
  return (n == p_node);
}

bool GameTreeNodeRep::IsSubgameRoot() const
{
  // First take care of a couple easy cases
  if (children.empty() || infoset->NumMembers() > 1) return false;
  if (!m_parent) return true;

  // A node is a subgame root if and only if in every information set,
  // either all members succeed the node in the tree,
  // or all members do not succeed the node in the tree.
  for (auto player : m_efg->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      bool precedes = infoset->GetMember(1)->IsSuccessorOf(const_cast<GameTreeNodeRep *>(this));
      for (int mem = 2; mem <= infoset->NumMembers(); mem++) {
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
  if (!m_parent) return;
  m_efg->IncrementVersion();
  GameTreeNodeRep *oldParent = m_parent;

  oldParent->children.Remove(oldParent->children.Find(this));
  oldParent->DeleteTree();
  m_parent = oldParent->m_parent;
  if (m_parent) {
    m_parent->children[m_parent->children.Find(oldParent)] = this;
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
  while (!children.empty()) {
    children.front()->DeleteTree();
    children.front()->Invalidate();
    children.Remove(1);
  }
  if (infoset) {
    infoset->RemoveMember(this);
    infoset = nullptr;
  }

  outcome = nullptr;
  m_label = "";

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

void GameTreeNodeRep::CopySubtree(GameTreeNodeRep *src, GameTreeNodeRep *stop)
{
  m_efg->IncrementVersion();
  if (src == stop) {
    outcome = src->outcome;
    return;
  }

  if (src->children.Length())  {
    AppendMove(src->infoset);
    for (int i = 1; i <= src->children.Length(); i++) {
      children[i]->CopySubtree(src->children[i], stop);
    }
  }

  m_label = src->m_label;
  outcome = src->outcome;
}

void GameTreeNodeRep::CopyTree(GameNode p_src)
{
  if (p_src->GetGame() != m_efg) throw MismatchException();
  if (p_src == this || !children.empty()) return;

  m_efg->IncrementVersion();
  auto *src = dynamic_cast<GameTreeNodeRep *>(p_src.operator->());

  if (!src->children.empty())  {
    AppendMove(src->infoset);
    for (int i = 1; i <= src->children.Length(); i++) {
      children[i]->CopySubtree(src->children[i], this);
    }

    m_efg->ClearComputedValues();
    m_efg->Canonicalize();
  }
}

void GameTreeNodeRep::MoveTree(GameNode p_src)
{
  if (p_src->GetGame() != m_efg) throw MismatchException();
  if (p_src == this || !children.empty() || IsSuccessorOf(p_src)) {
    return;
  }
  m_efg->IncrementVersion();
  auto *src = dynamic_cast<GameTreeNodeRep *>(p_src.operator->());

  if (src->m_parent == m_parent) {
    int srcChild = src->m_parent->children.Find(src);
    int destChild = src->m_parent->children.Find(this);
    src->m_parent->children[srcChild] = this;
    src->m_parent->children[destChild] = src;
  }
  else {
    GameTreeNodeRep *parent = src->m_parent; 
    parent->children[parent->children.Find(src)] = this;
    m_parent->children[m_parent->children.Find(this)] = src;
    src->m_parent = m_parent;
    m_parent = parent;
  }

  m_label = "";
  outcome = nullptr;
  
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
  if (p_infoset->GetGame() != m_efg) throw MismatchException();
  if (!infoset || infoset == p_infoset) return;
  if (p_infoset->NumActions() != children.Length()) 
    throw DimensionException();
  m_efg->IncrementVersion();
  infoset->RemoveMember(this);
  dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->())->AddMember(this);
  infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

GameInfoset GameTreeNodeRep::LeaveInfoset()
{
  if (!infoset) return nullptr;

  m_efg->IncrementVersion();
  GameTreeInfosetRep *oldInfoset = infoset;
  if (oldInfoset->m_members.Length() == 1) return oldInfoset;

  GamePlayerRep *player = oldInfoset->m_player;
  oldInfoset->RemoveMember(this);
  infoset = new GameTreeInfosetRep(m_efg, player->m_infosets.Length() + 1, player,
			       children.Length());
  infoset->AddMember(this);
  for (int i = 1; i <= oldInfoset->m_actions.Length(); i++) {
    infoset->m_actions[i]->SetLabel(oldInfoset->m_actions[i]->GetLabel());
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return infoset;
}

GameInfoset GameTreeNodeRep::AppendMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0 || !children.empty()) throw UndefinedException();
  if (p_player->GetGame() != m_efg) throw MismatchException();

  m_efg->IncrementVersion();
  return AppendMove(new GameTreeInfosetRep(m_efg, 
				       p_player->m_infosets.size() + 1,
				       p_player, p_actions));
}  

GameInfoset GameTreeNodeRep::AppendMove(GameInfoset p_infoset)
{
  if (!children.empty()) throw UndefinedException();
  if (p_infoset->GetGame() != m_efg) throw MismatchException();

  m_efg->IncrementVersion();
  infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());
  infoset->AddMember(this);
  for (int i = 1; i <= p_infoset->NumActions(); i++) {
    children.push_back(new GameTreeNodeRep(m_efg, this));
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return infoset;
}
  
GameInfoset GameTreeNodeRep::InsertMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0) throw UndefinedException();
  if (p_player->GetGame() != m_efg) throw MismatchException();

  m_efg->IncrementVersion();
  return InsertMove(new GameTreeInfosetRep(m_efg, 
				       p_player->m_infosets.Length() + 1, 
				       p_player, p_actions));
}

GameInfoset GameTreeNodeRep::InsertMove(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) throw MismatchException();

  m_efg->IncrementVersion();
  auto *newNode = new GameTreeNodeRep(m_efg, m_parent);
  newNode->infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());
  dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->())->AddMember(newNode);

  if (m_parent) {
    m_parent->children[m_parent->children.Find(this)] = newNode;
  }
  else {
    m_efg->m_root = newNode;
  }

  newNode->children.push_back(this);
  m_parent = newNode;

  for (int i = 1; i < p_infoset->NumActions(); i++) {
    newNode->children.push_back(new GameTreeNodeRep(m_efg, newNode));
  }

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
  : m_computedValues(false), m_doCanon(true)
{
  m_chance = new GamePlayerRep(this, 0);
  m_root = new GameTreeNodeRep(this, nullptr);
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

Game NewTree()  { return new GameTreeRep(); }

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
    for (int i = 2; i <= p_node->NumChildren(); i++) {
      if (SubtreeSum(p_node->GetChild(i)) != sum) {
	throw NotZeroSumException();
      }
    }
  }

  if (p_node->GetOutcome()) {
    for (int pl = 1; pl <= p_node->GetGame()->NumPlayers(); pl++) {
      sum += static_cast<Rational>(p_node->GetOutcome()->GetPayoff(pl));
    }
  }
  return sum;
}
 
}  // end anonymous namespace


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
    for (int i = 1; i <= player->NumInfosets(); i++) {
      GameTreeInfosetRep *iset1 = player->m_infosets[i];
      for (int j = 1; j <= player->NumInfosets(); j++) {
        GameTreeInfosetRep *iset2 = player->m_infosets[j];

        bool precedes = false;
        int action = 0;

        for (int m = 1; m <= iset2->NumMembers(); m++) {
          int n;
          for (n = 1; n <= iset1->NumMembers(); n++) {
            if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)) &&
                iset1->GetMember(n) != iset2->GetMember(m)) {
              precedes = true;
              for (int act = 1; act <= iset1->NumActions(); act++) {
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
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

void GameTreeRep::Canonicalize()
{
  if (!m_doCanon)  return;
  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);

  for (int pl = 0; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = (pl) ? m_players[pl] : m_chance;
    
    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      GameTreeInfosetRep *infoset = player->m_infosets[iset];
      for (int i = 1; i < infoset->m_members.Length(); i++) {
	for (int j = 1; j < infoset->m_members.Length() - i; j++) {
	  if (infoset->m_members[j+1]->number < infoset->m_members[j]->number) {
	    GameTreeNodeRep *tmp = infoset->m_members[j];
	    infoset->m_members[j] = infoset->m_members[j+1];
	    infoset->m_members[j+1] = tmp;
	  }
	}
      }
    }

    // Sort information sets by the smallest ID among their members
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int i = 1; i < player->m_infosets.Length(); i++) {
      for (int j = 1; j < player->m_infosets.Length() - i; j++) {
	int a = ((player->m_infosets[j+1]->m_members.Length()) ?
		 player->m_infosets[j+1]->m_members[1]->number : 0);
	int b = ((player->m_infosets[j]->m_members.Length()) ?
		 player->m_infosets[j]->m_members[1]->number : 0);

	if (a < b || b == 0) {
	  GameTreeInfosetRep *tmp = player->m_infosets[j];
	  player->m_infosets[j] = player->m_infosets[j+1];
	  player->m_infosets[j+1] = tmp;
	}
      }
    }

    // Reassign information set IDs
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      player->m_infosets[iset]->m_number = iset;
    }
  }
}

void GameTreeRep::ClearComputedValues() const
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    while (m_players[pl]->m_strategies.Length() > 0) {
      m_players[pl]->m_strategies.Remove(1)->Invalidate();
    }
  }

  m_computedValues = false;
}

void GameTreeRep::BuildComputedValues()
{
  if (m_computedValues) return;

  Canonicalize();

  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl]->MakeReducedStrats(m_root, nullptr);
  }

  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); 
	 m_players[pl]->m_strategies[st++]->m_id = id++);
  }

  m_computedValues = true;
}

//------------------------------------------------------------------------
//                  GameTreeRep: Writing data files
//------------------------------------------------------------------------

namespace {

std::string EscapeQuotes(const std::string &s)
{
  std::string ret;
  
  for (unsigned int i = 0; i < s.length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}

void PrintActions(std::ostream &p_stream, GameTreeInfosetRep *p_infoset)
{ 
  p_stream << "{ ";
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    p_stream << '"' << EscapeQuotes(p_infoset->GetAction(act)->GetLabel()) << "\" ";
    if (p_infoset->IsChanceInfoset()) {
      p_stream << static_cast<std::string>(p_infoset->GetActionProb(act)) << ' ';
    }
  }
  p_stream << "}";
}

void WriteEfgFile(std::ostream &f, GameTreeNodeRep *n)
{
  if (n->NumChildren() == 0)   {
    f << "t \"" << EscapeQuotes(n->GetLabel()) << "\" ";
    if (n->GetOutcome())  {
      f << n->GetOutcome()->GetNumber() << " \"" <<
	EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
	f << static_cast<std::string>(n->GetOutcome()->GetPayoff(pl));

	if (pl < n->GetGame()->NumPlayers()) {
	  f << ", ";
	}
	else {
	  f << " }\n";
	}
      }
    }
    else {
      f << "0\n";
    }
    return;
  }

  if (n->GetInfoset()->IsChanceInfoset()) {
    f << "c \"";
  }
  else {
    f << "p \"";
  }

  f << EscapeQuotes(n->GetLabel()) << "\" ";
  if (!n->GetInfoset()->IsChanceInfoset()) {
    f << n->GetInfoset()->GetPlayer()->GetNumber() << ' ';
  }
  f << n->GetInfoset()->GetNumber() << " \"" <<
    EscapeQuotes(n->GetInfoset()->GetLabel()) << "\" ";
  PrintActions(f, dynamic_cast<GameTreeInfosetRep *>(n->GetInfoset().operator->()));
  f << " ";
  if (n->GetOutcome())  {
    f << n->GetOutcome()->GetNumber() << " \"" <<
      EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
    f << "{ ";
    for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
      f << static_cast<std::string>(n->GetOutcome()->GetPayoff(pl));
      
      if (pl < n->GetGame()->NumPlayers())
	f << ", ";
      else
	f << " }\n";
    }
  }
  else
    f << "0\n";

  for (int i = 1; i <= n->NumChildren(); 
       WriteEfgFile(f, dynamic_cast<GameTreeNodeRep *>(n->GetChild(i++).operator->())));
}

} // end anonymous namespace

void GameTreeRep::WriteEfgFile(std::ostream &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, m_root);
}

void GameTreeRep::WriteEfgFile(std::ostream &p_file, const GameNode &p_root) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, 
		       dynamic_cast<GameTreeNodeRep *>(p_root.operator->()));
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

PVector<int> GameTreeRep::NumActions() const
{
  Array<int> foo(m_players.Length());
  int i;
  for (i = 1; i <= m_players.Length(); i++)
    foo[i] = m_players[i]->m_infosets.Length();

  PVector<int> bar(foo);
  for (i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->NumActions();
    }
  }

  return bar;
}  

PVector<int> GameTreeRep::NumMembers() const
{
  Array<int> foo(m_players.Length());

  for (int i = 1; i <= m_players.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }

  PVector<int> bar(foo);
  for (int i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->NumInfosets(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->NumMembers();
    }
  }

  return bar;
}

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
  GamePlayerRep *player = nullptr;
  player = new GamePlayerRep(this, m_players.size() + 1);
  m_players.push_back(player);
  for (int outc = 1; outc <= m_outcomes.Last(); outc++) {
    m_outcomes[outc]->m_payoffs.push_back(Number());
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
  for (auto player: m_players) {
    for (auto infoset : player->m_infosets) {
      if (index++ == p_index) {
        return infoset;
      }
    }
  }
  throw IndexException();
}

Array<int> GameTreeRep::NumInfosets() const
{
  Array<int> foo(m_players.Length());
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }
  return foo;
}

GameAction GameTreeRep::GetAction(int p_index) const
{
  int index = 1;
  for (auto player: m_players) {
    for (auto infoset : player->m_infosets) {
      for (auto action : infoset->m_actions) {
        if (index++ == p_index) {
          return action;
        }
      }
    }
  }
  throw IndexException();
}


//------------------------------------------------------------------------
//                        GameTreeRep: Outcomes
//------------------------------------------------------------------------

void GameTreeRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  IncrementVersion();
  m_root->DeleteOutcome(p_outcome);
  m_outcomes.Remove(m_outcomes.Find(p_outcome))->Invalidate();
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_number = outc;
  }
  ClearComputedValues();
}

//------------------------------------------------------------------------
//                         GameTreeRep: Nodes
//------------------------------------------------------------------------

namespace {
int CountNodes(GameNode p_node)
{
  int num = 1;
  for (int i = 1; i <= p_node->NumChildren(); 
       num += CountNodes(p_node->GetChild(i++)));
  return num;
}

}  // end anonymous namespace

int GameTreeRep::NumNodes() const
{
  return CountNodes(m_root);
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
    throw UndefinedException("Action probabilities can only be specified for chance information sets");
  }
  if (p_infoset->NumActions() != p_probs.size()) {
    throw DimensionException("The number of probabilities given must match the number of actions");
  }
  IncrementVersion();
  Rational sum(0);
  for (auto prob : p_probs) {
    if (static_cast<Rational>(prob) < Rational(0)) {
      throw ValueException("Probabilities must be non-negative numbers");
    }
    sum += static_cast<Rational>(prob);
  }
  if (sum != Rational(1)) {
    throw ValueException("Probabilities must sum to exactly one");
  }
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    dynamic_cast<GameTreeInfosetRep &>(*p_infoset).m_probs[act] = p_probs[act];
  }
  ClearComputedValues();
  return this;
}

Game GameTreeRep::NormalizeChanceProbs(const GameInfoset &m_infoset)
{
  if (m_infoset->GetGame() != this) {
    throw MismatchException();
  }
  if (!m_infoset->IsChanceInfoset()) {
    throw UndefinedException(
      "Action probabilities can only be normalized for chance information sets");
  }
  IncrementVersion();
  Rational sum(0);
  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    Rational action_prob(m_infoset->GetActionProb(act));
    sum += action_prob;
  }
  Array<Number> m_probs(m_infoset->NumActions());
  if (sum == Rational(0)) {
    // all remaining moves have prob zero; split prob 1 equally among them
    for (int act = 1; act <= m_infoset->NumActions(); act++) {
      m_probs[act] = Rational(1, m_infoset->NumActions());
    }
  }
  else {
    for (int act = 1; act <= m_infoset->NumActions(); act++) {
      Rational prob(m_infoset->GetActionProb(act));
      m_probs[act] = prob / sum;
    }
  }
  m_infoset->GetGame()->SetChanceProbs(m_infoset, m_probs);
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
  return StrategySupportProfile(const_cast<GameTreeRep *>(this)).NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double> GameTreeRep::NewMixedStrategyProfile(double, const StrategySupportProfile& spt) const
{
  if (!IsPerfectRecall()) {
    throw UndefinedException("Mixed strategies not supported for games with imperfect recall.");
  }    
  return MixedStrategyProfile<double>(new TreeMixedStrategyProfileRep<double>(spt));
}

MixedStrategyProfile<Rational> GameTreeRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile& spt) const
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
  PureStrategyProfileRep *Copy() const override;

public:
  TreePureStrategyProfileRep(const Game &p_game)
    : PureStrategyProfileRep(p_game) { }
  void SetStrategy(const GameStrategy &) override;
  GameOutcome GetOutcome() const override
  { throw UndefinedException(); }
  void SetOutcome(GameOutcome p_outcome) override
  { throw UndefinedException(); }
  Rational GetPayoff(int pl) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//              TreePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

PureStrategyProfileRep *TreePureStrategyProfileRep::Copy() const
{
  return new TreePureStrategyProfileRep(*this);
}

PureStrategyProfile GameTreeRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(new TreePureStrategyProfileRep(const_cast<GameTreeRep *>(this)));
}

//------------------------------------------------------------------------
//       TreePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void TreePureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Rational TreePureStrategyProfileRep::GetPayoff(int pl) const
{
  PureBehaviorProfile behav(m_nfg);
  for (int i = 1; i <= m_nfg->NumPlayers(); i++) {
    GamePlayer player = m_nfg->GetPlayer(i);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      int act = m_profile[i]->m_behav[iset];
      if (act) {
	behav.SetAction(player->GetInfoset(iset)->GetAction(act));
      }
    }
  }
  return behav.GetPayoff<Rational>(pl);
}

Rational
TreePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  PureStrategyProfile copy(Copy());
  copy->SetStrategy(p_strategy);
  return copy->GetPayoff(p_strategy->GetPlayer()->GetNumber());
}


}  // end namespace Gambit
