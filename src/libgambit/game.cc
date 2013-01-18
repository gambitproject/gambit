//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/game.cc
// Implementation of extensive form game representation
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
#include <sstream>

#include "libgambit.h"
#include "gametree.h"
#include "gametable.h"

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number)
  : m_game(p_game), m_number(p_number),
    m_payoffs(m_game->NumPlayers())
{ }

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

void GameTreeActionRep::DeleteAction(void)
{
  if (m_infoset->NumActions() == 1) throw UndefinedException();

  int where;
  for (where = 1;
       where <= m_infoset->m_actions.Length() && 
	 m_infoset->m_actions[where] != this;
       where++);

  m_infoset->RemoveAction(where);
  for (int i = 1; i <= m_infoset->m_members.Length(); i++)   {
    m_infoset->m_members[i]->children[where]->DeleteTree();
    m_infoset->m_members[i]->children.Remove(where)->Invalidate();
  }
  m_infoset->m_efg->ClearComputedValues();
  m_infoset->m_efg->Canonicalize();
}

GameInfoset GameTreeActionRep::GetInfoset(void) const { return m_infoset; }

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

  m_player->m_infosets.Append(this);

  if (p_player->IsChance()) {
    for (int act = 1; act <= m_actions.Length(); act++) {
      m_probs.Append(lexical_cast<std::string>(Rational(1, m_actions.Length())));
    }
  }
}

GameTreeInfosetRep::~GameTreeInfosetRep()  
{
  for (int act = 1; act <= m_actions.Length(); m_actions[act++]->Invalidate());
}

void GameTreeInfosetRep::SetPlayer(GamePlayer p_player)
{
  if (p_player->GetGame() != m_efg) throw MismatchException();
  if (m_player->IsChance() || p_player->IsChance()) throw UndefinedException();
  if (m_player == p_player) return;

  m_player->m_infosets.Remove(m_player->m_infosets.Find(this));
  m_player = p_player;
  p_player->m_infosets.Append(this);

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

bool GameTreeInfosetRep::Precedes(GameNode p_node) const
{
  GameTreeNodeRep *node = dynamic_cast<GameTreeNodeRep *>(p_node.operator->());
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
  
  int where = m_actions.Length() + 1;
  if (p_action) {
    for (where = 1; m_actions[where] != p_action; where++); 
  }

  GameTreeActionRep *action = new GameTreeActionRep(where, "", this);
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
  m_actions.Remove(which)->Invalidate();
  for (; which <= m_actions.Length(); which++)
    m_actions[which]->m_number = which;

  if (m_player->IsChance()) {
    m_probs.Remove(which);
  }
}

void GameTreeInfosetRep::SetActionProb(int act, const std::string &p_value)
{
  m_probs[act] = p_value;
  m_efg->ClearComputedValues();
}

void GameTreeInfosetRep::RemoveMember(GameTreeNodeRep *p_node)
{
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
  for (int act = 1; act <= m_actions.Length(); act++) {
    GameActionRep *action = m_actions[act];
    for (int iset = 1; iset <= p_player->m_infosets.Length(); iset++) {
      // make copy of members to iterate correctly 
      // (since the information set may be changed in the process)
      Array<GameTreeNodeRep *> members = p_player->m_infosets[iset]->m_members;

      // This information set holds all members of information set
      // which follow 'action'.
      GameInfoset newiset = 0;

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
{ return m_members[p_index]; }

GamePlayer GameTreeInfosetRep::GetPlayer(void) const { return m_player; }

bool GameTreeInfosetRep::IsChanceInfoset(void) const
{ return m_player->IsChance(); }

//========================================================================
//                      class GameStrategyRep
//========================================================================

void GameStrategyRep::DeleteStrategy(void)
{
  if (m_player->GetGame()->IsTree())  throw UndefinedException();
  if (m_player->NumStrategies() == 1)  return;

  m_player->m_strategies.Remove(m_player->m_strategies.Find(this));
  for (int st = 1; st <= m_player->m_strategies.Length(); st++) {
    m_player->m_strategies[st]->m_number = st;
  }
  //m_player->m_game->RebuildTable();
  this->Invalidate();
}

//========================================================================
//                       class GamePlayerRep
//========================================================================

GamePlayerRep::GamePlayerRep(GameRep *p_game, int p_id, int p_strats)
  : m_game(p_game), m_number(p_id), m_strategies(p_strats)
{ 
  for (int j = 1; j <= p_strats; j++) {
    m_strategies[j] = new GameStrategyRep(this);
    m_strategies[j]->m_number = j;
  }
}

GamePlayerRep::~GamePlayerRep()
{ 
  for (int j = 1; j <= m_infosets.Length(); m_infosets[j++]->Invalidate());
  for (int j = 1; j <= m_strategies.Length(); m_strategies[j++]->Invalidate());
}


GameStrategy GamePlayerRep::NewStrategy(void)
{
  if (m_game->IsTree())  throw UndefinedException();

  GameStrategyRep *strategy = new GameStrategyRep(this);
  m_strategies.Append(strategy);
  strategy->m_number = m_strategies.Length();
  strategy->m_offset = -1;   // this flags this action as new
  dynamic_cast<GameTableRep *>(m_game)->RebuildTable();
  return strategy;
}

void GamePlayerRep::MakeStrategy(void)
{
  Array<int> c(NumInfosets());
  
  for (int i = 1; i <= NumInfosets(); i++)  {
    if (m_infosets[i]->flag == 1)
      c[i] = m_infosets[i]->whichbranch;
    else
      c[i] = 0;
  }
  
  GameStrategyRep *strategy = new GameStrategyRep(this);
  m_strategies.Append(strategy);
  strategy->m_number = m_strategies.Length();
  strategy->m_behav = c;
  strategy->m_label = "";

  // We generate a default labeling -- probably should be changed in future
  if (strategy->m_behav.Length() > 0) {
    for (int iset = 1; iset <= strategy->m_behav.Length(); iset++) {
      if (strategy->m_behav[iset] > 0) {
	strategy->m_label += lexical_cast<std::string>(strategy->m_behav[iset]);
      }
      else {
	strategy->m_label += "*";
      }
    }
  }
  else {
    strategy->m_label = "*";
  }
}

void GamePlayerRep::MakeReducedStrats(GameTreeNodeRep *n, GameTreeNodeRep *nn)
{
  int i;
  GameTreeNodeRep *m, *mm;

  if (!n->GetParent())  n->ptr = 0;

  if (n->NumChildren() > 0)  {
    if (n->infoset->m_player == this)  {
      if (n->infoset->flag == 0)  {
	// we haven't visited this infoset before
	n->infoset->flag = 1;
	for (i = 1; i <= n->NumChildren(); i++)   {
	  GameTreeNodeRep *m = n->children[i];
	  n->whichbranch = m;
	  n->infoset->whichbranch = i;
	  MakeReducedStrats(m, nn);
	}
	n->infoset->flag = 0;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(n->children[n->infoset->whichbranch], nn);
      }
    }
    else  {
      n->ptr = NULL;
      if (nn != NULL)
	n->ptr = nn->m_parent;
      n->whichbranch = n->children[1];
      if (n->infoset)
	n->infoset->whichbranch = 0;
      MakeReducedStrats(n->children[1], n->children[1]);
    }
  }
  else if (nn)  {
    for (; ; nn = nn->m_parent->ptr->whichbranch)  {
      if (!nn->GetNextSibling()) {
	m = 0;
      }
      else {
	m = dynamic_cast<GameTreeNodeRep *>(nn->GetNextSibling().operator->());
      }
      if (m || nn->m_parent->ptr == NULL)   break;
    }
    if (m)  {
      mm = m->m_parent->whichbranch;
      m->m_parent->whichbranch = m;
      MakeReducedStrats(m, m);
      m->m_parent->whichbranch = mm;
    }
    else {
      MakeStrategy();
    }
  }
  else {
    MakeStrategy();
  }
}

GameInfoset GamePlayerRep::GetInfoset(int p_index) const { return m_infosets[p_index]; }

//========================================================================
//                         class GameTreeNodeRep
//========================================================================

GameTreeNodeRep::GameTreeNodeRep(GameTreeRep *e, GameTreeNodeRep *p)
  : number(0), m_efg(e), infoset(0), m_parent(p), outcome(0)
{ }

GameTreeNodeRep::~GameTreeNodeRep()
{
  for (int i = children.Length(); i; children[i--]->Invalidate());
}

GameNode GameTreeNodeRep::GetNextSibling(void) const  
{
  if (!m_parent)   return 0;
  if (m_parent->children.Find(const_cast<GameTreeNodeRep *>(this)) == 
      m_parent->children.Length())
    return 0;
  else
    return m_parent->children[m_parent->children.Find(const_cast<GameTreeNodeRep *>(this)) + 1];
}

GameNode GameTreeNodeRep::GetPriorSibling(void) const
{ 
  if (!m_parent)   return 0;
  if (m_parent->children.Find(const_cast<GameTreeNodeRep *>(this)) == 1)
    return 0;
  else
    return m_parent->children[m_parent->children.Find(const_cast<GameTreeNodeRep *>(this)) - 1];

}

GameAction GameTreeNodeRep::GetPriorAction(void) const
{
  if (!m_parent) {
    return 0;
  }
  
  GameTreeInfosetRep *infoset = m_parent->infoset;
  for (int i = 1; i <= infoset->NumActions(); i++) {
    if (GameNode(const_cast<GameTreeNodeRep *>(this)) == GetParent()->GetChild(i)) {
      return infoset->GetAction(i);
    }
  }

  return 0;
}

void GameTreeNodeRep::DeleteOutcome(GameOutcomeRep *outc)
{
  if (outc == outcome)   outcome = 0;
  for (int i = 1; i <= children.Length(); i++)
    children[i]->DeleteOutcome(outc);
}

void GameTreeNodeRep::SetOutcome(const GameOutcome &p_outcome)
{
  if (p_outcome != outcome) {
    outcome = p_outcome;
    m_efg->ClearComputedValues();
  }
}

bool GameTreeNodeRep::IsSuccessorOf(GameNode p_node) const
{
  GameTreeNodeRep *n = const_cast<GameTreeNodeRep *>(this);
  while (n && n != p_node) n = n->m_parent;
  return (n == p_node);
}

bool GameTreeNodeRep::IsSubgameRoot(void) const
{
  // First take care of a couple easy cases
  if (children.Length() == 0 || infoset->NumMembers() > 1) return false;
  if (!m_parent) return true;

  // A node is a subgame root if and only if in every information set,
  // either all members succeed the node in the tree,
  // or all members do not succeed the node in the tree.
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    GamePlayerRep *player = m_efg->GetPlayer(pl);
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameTreeInfosetRep *infoset = dynamic_cast<GameTreeInfosetRep *>(player->GetInfoset(iset).operator->());

      bool precedes = infoset->GetMember(1)->IsSuccessorOf(const_cast<GameTreeNodeRep *>(this));

      for (int mem = 2; mem <= infoset->NumMembers(); mem++) {
	if (infoset->GetMember(mem)->IsSuccessorOf(const_cast<GameTreeNodeRep *>(this)) != precedes) {
	  return false;
	}
      }
    }
  }

  return true;
}

void GameTreeNodeRep::DeleteParent(void)
{
  if (!m_parent) return;
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

void GameTreeNodeRep::DeleteTree(void)
{
  for (int i = 1; i <= children.Length(); i++) {
    children[i]->DeleteTree();
    children[i]->Invalidate();
  }
  children = Array<GameTreeNodeRep *>();

  if (infoset) {
    infoset->RemoveMember(this);
    infoset = 0;
  }

  outcome = 0;
  m_label = "";

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

void GameTreeNodeRep::CopySubtree(GameTreeNodeRep *src, GameTreeNodeRep *stop)
{
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
  if (p_src == this || children.Length() > 0) return;

  GameTreeNodeRep *src = dynamic_cast<GameTreeNodeRep *>(p_src.operator->());

  if (src->children.Length())  {
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
  if (p_src == this || children.Length() > 0 || IsSuccessorOf(p_src)) {
    return;
  }

  GameTreeNodeRep *src = dynamic_cast<GameTreeNodeRep *>(p_src.operator->());

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
  outcome = 0;
  
  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

void GameTreeNodeRep::SetInfoset(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) throw MismatchException();
  if (!infoset || infoset == p_infoset) return;
  if (p_infoset->NumActions() != children.Length()) 
    throw MismatchException();

  infoset->RemoveMember(this);
  dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->())->AddMember(this);
  infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
}

GameInfoset GameTreeNodeRep::LeaveInfoset(void)
{
  if (!infoset) return 0;

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
  if (p_actions <= 0 || children.Length() > 0) throw UndefinedException();
  if (p_player->GetGame() != m_efg) throw MismatchException();

  return AppendMove(new GameTreeInfosetRep(m_efg, 
				       p_player->m_infosets.Length() + 1, 
				       p_player, p_actions));
}  

GameInfoset GameTreeNodeRep::AppendMove(GameInfoset p_infoset)
{
  if (children.Length() > 0) throw UndefinedException();
  if (p_infoset->GetGame() != m_efg) throw MismatchException();
  
  infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());
  infoset->AddMember(this);
  for (int i = 1; i <= p_infoset->NumActions(); i++) {
    children.Append(new GameTreeNodeRep(m_efg, this));
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return infoset;
}
  
GameInfoset GameTreeNodeRep::InsertMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0) throw UndefinedException();
  if (p_player->GetGame() != m_efg) throw MismatchException();

  return InsertMove(new GameTreeInfosetRep(m_efg, 
				       p_player->m_infosets.Length() + 1, 
				       p_player, p_actions));
}

GameInfoset GameTreeNodeRep::InsertMove(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) throw MismatchException();

  GameTreeNodeRep *newNode = new GameTreeNodeRep(m_efg, m_parent);
  newNode->infoset = dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->());
  dynamic_cast<GameTreeInfosetRep *>(p_infoset.operator->())->AddMember(newNode);

  if (m_parent) {
    m_parent->children[m_parent->children.Find(this)] = newNode;
  }
  else {
    m_efg->m_root = newNode;
  }

  newNode->children.Append(this);
  m_parent = newNode;

  for (int i = 1; i < p_infoset->NumActions(); i++) {
    newNode->children.Append(new GameTreeNodeRep(m_efg, newNode));
  }

  m_efg->ClearComputedValues();
  m_efg->Canonicalize();
  return p_infoset;
}


//========================================================================
//                       class PureBehavProfile
//========================================================================

//------------------------------------------------------------------------
//                     PureBehavProfile: Lifecycle
//------------------------------------------------------------------------

PureBehavProfile::PureBehavProfile(Game p_efg)
  : m_efg(p_efg), m_profile(m_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    GamePlayerRep *player = m_efg->GetPlayer(pl);
    m_profile[pl] = Array<GameAction>(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_profile[pl][iset] = player->GetInfoset(iset)->GetAction(1);
    }
  }
}

//------------------------------------------------------------------------
//              PureBehavProfile: Data access and manipulation
//------------------------------------------------------------------------

GameAction PureBehavProfile::GetAction(const GameInfoset &infoset) const
{
  return m_profile[infoset->GetPlayer()->GetNumber()][infoset->GetNumber()];
}

void PureBehavProfile::SetAction(const GameAction &action)
{
  m_profile[action->GetInfoset()->GetPlayer()->GetNumber()]
    [action->GetInfoset()->GetNumber()] = action;
}

template <class T> 
T PureBehavProfile::GetNodeValue(const GameNode &p_node, 
				 int pl) const
{
  T payoff(0);

  GameTreeNodeRep *node = dynamic_cast<GameTreeNodeRep *>(p_node.operator->());

  if (node->outcome) {
    payoff += node->outcome->GetPayoff<T>(pl);
  }

  if (!node->IsTerminal()) {
    if (node->GetInfoset()->IsChanceInfoset()) {
      for (int i = 1; i <= node->NumChildren(); i++) {
	GameTreeInfosetRep *infoset = node->infoset;
	payoff += (infoset->GetActionProb(i, (T) 0) *
		   GetNodeValue<T>(node->children[i], pl));
      }
    }
    else {
      int player = node->GetPlayer()->GetNumber();
      int iset = node->GetInfoset()->GetNumber();
      payoff += GetNodeValue<T>(node->children[m_profile[player][iset]->GetNumber()], 
				pl);
    }
  }

  return payoff;
}

// Explicit instantiations
template double PureBehavProfile::GetNodeValue(const GameNode &, int pl) const;
template Rational PureBehavProfile::GetNodeValue(const GameNode &, int pl) const;

template <class T>
T PureBehavProfile::GetActionValue(const GameAction &p_action) const
{
  PureBehavProfile copy(*this);
  copy.SetAction(p_action);
  return copy.GetPayoff<T>(p_action->GetInfoset()->GetPlayer()->GetNumber());
}

// Explicit instantiations
template double PureBehavProfile::GetActionValue(const GameAction &) const;
template Rational PureBehavProfile::GetActionValue(const GameAction &) const;

//========================================================================
//                       class GameExplicitRep
//========================================================================


//------------------------------------------------------------------------
//                     GameExplicitRep: Lifecycle
//------------------------------------------------------------------------

GameExplicitRep::~GameExplicitRep()
{
  for (int pl = 1; pl <= m_players.Length(); m_players[pl++]->Invalidate());
  for (int outc = 1; outc <= m_outcomes.Length(); 
       m_outcomes[outc++]->Invalidate());
}

//------------------------------------------------------------------------
//                     GameRep: General data access
//------------------------------------------------------------------------

Rational GameExplicitRep::GetMinPayoff(int player) const
{
  int index, p, p1, p2;
  
  if (m_outcomes.Length() == 0)  return Rational(0);

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }
  
  Rational minpay = m_outcomes[1]->GetPayoff<Rational>(p1);
  for (index = 1; index <= m_outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++) {
      if (m_outcomes[index]->GetPayoff<Rational>(p) < minpay) {
	minpay = m_outcomes[index]->GetPayoff<Rational>(p);
      }
    }
  }
  return minpay;
}

Rational GameExplicitRep::GetMaxPayoff(int player) const
{
  int index, p, p1, p2;

  if (m_outcomes.Length() == 0)  return Rational(0);

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }

  Rational maxpay = m_outcomes[1]->GetPayoff<Rational>(p1);
  for (index = 1; index <= m_outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++)
      if (m_outcomes[index]->GetPayoff<Rational>(p) > maxpay)
	maxpay = m_outcomes[index]->GetPayoff<Rational>(p);
  }
  return maxpay;
}

//------------------------------------------------------------------------
//                GameExplicitRep: Dimensions of the game
//------------------------------------------------------------------------

Array<int> GameExplicitRep::NumStrategies(void) const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  Array<int> dim(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    dim[pl] = m_players[pl]->m_strategies.Length();
  }
  return dim;
}

GameStrategy GameExplicitRep::GetStrategy(int p_index) const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  for (int pl = 1, i = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); st++, i++) {
      if (p_index == i) {
	return m_players[pl]->m_strategies[st];
      }
    }
  }
  throw IndexException();
}

int GameExplicitRep::MixedProfileLength(void) const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  int strats = 0;
  for (int i = 1; i <= m_players.Length();
       strats += m_players[i++]->m_strategies.Length());
  return strats;
}


//------------------------------------------------------------------------
//                      GameExplicitRep: Outcomes
//------------------------------------------------------------------------

GameOutcome GameExplicitRep::NewOutcome(void)
{
  m_outcomes.Append(new GameOutcomeRep(this, m_outcomes.Length() + 1));
  return m_outcomes[m_outcomes.Last()];
}


// Deferred as this requires definition of GameTableRep
Game GameTreeNodeRep::GetGame(void) const { return m_efg; }
Game GameTreeInfosetRep::GetGame(void) const { return m_efg; }

}  // end namespace Gambit
