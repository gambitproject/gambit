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

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number)
  : m_game(p_game), m_number(p_number),
    m_payoffs(m_game->NumPlayers())
{ }

//========================================================================
//                       class GameActionRep
//========================================================================

bool GameActionRep::Precedes(const GameNode &n) const
{
  GameNode node = n;

  while (node != node->GetGame()->GetRoot()) {
    if (node->GetPriorAction() == this) {
      return true;
    }
    else {
      node = node->GetParent();
    }
  }
  return false;
}

void GameActionRep::DeleteAction(void)
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
}

//========================================================================
//                       class GameInfosetRep
//========================================================================

GameInfosetRep::GameInfosetRep(GameRep *p_efg, int p_number,
			       GamePlayerRep *p_player, int p_actions)
  : m_efg(p_efg), m_number(p_number), m_player(p_player), 
    m_actions(p_actions), flag(0) 
{
  while (p_actions)   {
    m_actions[p_actions] = new GameActionRep(p_actions, "", this);
    p_actions--; 
  }

  m_player->m_infosets.Append(this);

  if (p_player->IsChance()) {
    for (int act = 1; act <= m_actions.Length(); act++) {
      m_probs.Append(lexical_cast<std::string>(Rational(1, m_actions.Length())));
    }
  }
}

GameInfosetRep::~GameInfosetRep()  
{
  for (int act = 1; act <= m_actions.Length(); m_actions[act++]->Invalidate());
}

void GameInfosetRep::SetPlayer(GamePlayer p_player)
{
  if (p_player->GetGame() != m_efg) throw MismatchException();
  if (m_player->IsChance() || p_player->IsChance()) throw UndefinedException();
  if (m_player == p_player) return;

  m_player->m_infosets.Remove(m_player->m_infosets.Find(this));
  m_player = p_player;
  p_player->m_infosets.Append(this);

  m_efg->ClearComputedValues();
}

bool GameInfosetRep::Precedes(GameNode p_node) const
{
  while (p_node != p_node->GetGame()->GetRoot()) {
    if (p_node->GetInfoset() == this) {
      return true;
    }
    else {
      p_node = p_node->GetParent();
    }
  }
  return false;
}

GameAction GameInfosetRep::InsertAction(GameAction p_action /* =0 */)
{
  if (p_action && p_action->GetInfoset() != this) throw MismatchException();
  
  int where = m_actions.Length() + 1;
  if (p_action) {
    for (where = 1; m_actions[where] != p_action; where++); 
  }

  GameActionRep *action = new GameActionRep(where, "", this);
  m_actions.Insert(action, where);
  if (m_player->IsChance()) {
    m_probs.Insert(Number("0"), where);
  }

  for (int act = 1; act <= m_actions.Length(); act++) {
    m_actions[act]->m_number = act;
  }

  for (int i = 1; i <= m_members.Length(); i++) {
    m_members[i]->children.Insert(new GameNodeRep(m_efg, m_members[i]), 
				  where);
  }

  m_efg->ClearComputedValues();
  return action;
}


void GameInfosetRep::RemoveAction(int which)
{
  m_actions.Remove(which)->Invalidate();
  for (; which <= m_actions.Length(); which++)
    m_actions[which]->m_number = which;

  if (m_player->IsChance()) {
    m_probs.Remove(which);
  }
}

void GameInfosetRep::SetActionProb(int act, const std::string &p_value)
{
  m_probs[act] = p_value;
  m_efg->ClearComputedValues();
}

void GameInfosetRep::RemoveMember(GameNodeRep *p_node)
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

void GameInfosetRep::Reveal(GamePlayer p_player)
{
  for (int act = 1; act <= m_actions.Length(); act++) {
    GameActionRep *action = m_actions[act];
    for (int iset = 1; iset <= p_player->m_infosets.Length(); iset++) {
      // make copy of members to iterate correctly 
      // (since the information set may be changed in the process)
      Array<GameNodeRep *> members = p_player->m_infosets[iset]->m_members;

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
}

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
  m_game->RebuildTable();
  return strategy;
}

void GamePlayerRep::MakeStrategy(void)
{
  Array<int> c(NumInfosets());
  
  for (int i = 1; i <= NumInfosets(); i++)  {
    if (GetInfoset(i)->flag == 1)
      c[i] = GetInfoset(i)->whichbranch;
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

void GamePlayerRep::MakeReducedStrats(GameNodeRep *n, GameNodeRep *nn)
{
  int i;
  GameNodeRep *m, *mm;

  if (!n->GetParent())  n->ptr = 0;

  if (n->NumChildren() > 0)  {
    if (n->infoset->m_player == this)  {
      if (n->infoset->flag == 0)  {
	// we haven't visited this infoset before
	n->infoset->flag = 1;
	for (i = 1; i <= n->NumChildren(); i++)   {
	  GameNodeRep *m = n->GetChild(i);
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
      m = nn->GetNextSibling();
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

//========================================================================
//                         class GameNodeRep
//========================================================================

GameNodeRep::GameNodeRep(GameRep *e, GameNodeRep *p)
  : number(0), m_efg(e), infoset(0), m_parent(p), outcome(0)
{ }

GameNodeRep::~GameNodeRep()
{
  for (int i = children.Length(); i; children[i--]->Invalidate());
}


GameNode GameNodeRep::GetNextSibling(void) const  
{
  if (!m_parent)   return 0;
  if (m_parent->children.Find(const_cast<GameNodeRep *>(this)) == 
      m_parent->children.Length())
    return 0;
  else
    return m_parent->children[m_parent->children.Find(const_cast<GameNodeRep *>(this)) + 1];
}

GameNode GameNodeRep::GetPriorSibling(void) const
{ 
  if (!m_parent)   return 0;
  if (m_parent->children.Find(const_cast<GameNodeRep *>(this)) == 1)
    return 0;
  else
    return m_parent->children[m_parent->children.Find(const_cast<GameNodeRep *>(this)) - 1];

}

GameAction GameNodeRep::GetPriorAction(void) const
{
  if (!m_parent) {
    return 0;
  }
  
  GameInfosetRep *infoset = GetParent()->GetInfoset();
  for (int i = 1; i <= infoset->NumActions(); i++) {
    if (this == GetParent()->GetChild(i)) {
      return infoset->GetAction(i);
    }
  }

  return 0;
}

void GameNodeRep::DeleteOutcome(GameOutcomeRep *outc)
{
  if (outc == outcome)   outcome = 0;
  for (int i = 1; i <= children.Length(); i++)
    children[i]->DeleteOutcome(outc);
}

void GameNodeRep::SetOutcome(const GameOutcome &p_outcome)
{
  if (p_outcome != outcome) {
    outcome = p_outcome;
    m_efg->ClearComputedValues();
  }
}

bool GameNodeRep::IsSuccessorOf(GameNode p_node) const
{
  GameNode n = const_cast<GameNodeRep *>(this);
  while (n && n != p_node) n = n->m_parent;
  return (n == p_node);
}

bool GameNodeRep::IsSubgameRoot(void) const
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
      GameInfosetRep *infoset = player->GetInfoset(iset);

      bool precedes = infoset->GetMember(1)->IsSuccessorOf(const_cast<GameNodeRep *>(this));

      for (int mem = 2; mem <= infoset->NumMembers(); mem++) {
	if (infoset->GetMember(mem)->IsSuccessorOf(const_cast<GameNodeRep *>(this)) != precedes) {
	  return false;
	}
      }
    }
  }

  return true;
}

void GameNodeRep::DeleteParent(void)
{
  if (!m_parent) return;
  GameNodeRep *oldParent = m_parent;

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
}

void GameNodeRep::DeleteTree(void)
{
  for (int i = 1; i <= children.Length(); i++) {
    children[i]->DeleteTree();
    children[i]->Invalidate();
  }
  children = Array<GameNodeRep *>();

  if (infoset) {
    infoset->RemoveMember(this);
    infoset = 0;
  }

  outcome = 0;
  m_label = "";

  m_efg->ClearComputedValues();
}

void GameNodeRep::CopySubtree(GameNodeRep *src, GameNodeRep *stop)
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

void GameNodeRep::CopyTree(GameNode p_src)
{
  if (p_src->GetGame() != m_efg) throw MismatchException();
  if (p_src == this || children.Length() > 0) return;

  if (p_src->children.Length())  {
    AppendMove(p_src->infoset);
    for (int i = 1; i <= p_src->children.Length(); i++) {
      children[i]->CopySubtree(p_src->children[i], this);
    }

    m_efg->ClearComputedValues();
  }
}

void GameNodeRep::MoveTree(GameNode p_src)
{
  if (p_src->GetGame() != m_efg) throw MismatchException();
  if (p_src == this || children.Length() > 0 || IsSuccessorOf(p_src)) {
    return;
  }

  if (p_src->m_parent == m_parent) {
    int srcChild = p_src->m_parent->children.Find(p_src);
    int destChild = p_src->m_parent->children.Find(this);
    p_src->m_parent->children[srcChild] = this;
    p_src->m_parent->children[destChild] = p_src;
  }
  else {
    GameNodeRep *parent = p_src->m_parent; 
    parent->children[parent->children.Find(p_src)] = this;
    m_parent->children[m_parent->children.Find(this)] = p_src;
    p_src->m_parent = m_parent;
    m_parent = parent;
  }

  m_label = "";
  outcome = 0;
  
  m_efg->ClearComputedValues();
}

void GameNodeRep::SetInfoset(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) throw MismatchException();
  if (!infoset || infoset == p_infoset) return;
  if (p_infoset->m_actions.Length() != children.Length()) 
    throw MismatchException();

  infoset->RemoveMember(this);
  p_infoset->AddMember(this);
  infoset = p_infoset;

  m_efg->ClearComputedValues();
}

GameInfoset GameNodeRep::LeaveInfoset(void)
{
  if (!infoset) return 0;

  GameInfosetRep *oldInfoset = infoset;
  if (oldInfoset->m_members.Length() == 1) return oldInfoset;

  GamePlayerRep *player = oldInfoset->m_player;
  oldInfoset->RemoveMember(this);
  infoset = new GameInfosetRep(m_efg, player->m_infosets.Length() + 1, player,
			       children.Length());
  infoset->AddMember(this);
  for (int i = 1; i <= oldInfoset->m_actions.Length(); i++) {
    infoset->m_actions[i]->SetLabel(oldInfoset->m_actions[i]->GetLabel());
  }

  m_efg->ClearComputedValues();
  return infoset;
}

GameInfoset GameNodeRep::AppendMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0 || children.Length() > 0) throw UndefinedException();
  if (p_player->GetGame() != m_efg) throw MismatchException();

  return AppendMove(new GameInfosetRep(m_efg, 
				       p_player->m_infosets.Length() + 1, 
				       p_player, p_actions));
}  

GameInfoset GameNodeRep::AppendMove(GameInfoset p_infoset)
{
  if (children.Length() > 0) throw UndefinedException();
  if (p_infoset->GetGame() != m_efg) throw MismatchException();
  
  infoset = p_infoset;
  infoset->AddMember(this);
  for (int i = 1; i <= p_infoset->NumActions(); i++) {
    children.Append(new GameNodeRep(m_efg, this));
  }

  m_efg->ClearComputedValues();
  return infoset;
}
  
GameInfoset GameNodeRep::InsertMove(GamePlayer p_player, int p_actions)
{
  if (p_actions <= 0) throw UndefinedException();
  if (p_player->GetGame() != m_efg) throw MismatchException();

  return InsertMove(new GameInfosetRep(m_efg, 
				       p_player->m_infosets.Length() + 1, 
				       p_player, p_actions));
}

GameInfoset GameNodeRep::InsertMove(GameInfoset p_infoset)
{
  if (p_infoset->GetGame() != m_efg) throw MismatchException();

  GameNodeRep *newNode = new GameNodeRep(m_efg, m_parent);
  newNode->infoset = p_infoset;
  p_infoset->AddMember(newNode);

  if (m_parent) {
    m_parent->children[m_parent->children.Find(this)] = newNode;
  }
  else {
    m_efg->m_root = newNode;
  }

  newNode->children.Append(this);
  m_parent = newNode;

  for (int i = 1; i < p_infoset->NumActions(); i++) {
    newNode->children.Append(new GameNodeRep(m_efg, newNode));
  }

  m_efg->ClearComputedValues();
  return p_infoset;
}

//========================================================================
//                      class PureStrategyProfile
//========================================================================

//------------------------------------------------------------------------
//                    PureStrategyProfile: Lifecycle
//------------------------------------------------------------------------

PureStrategyProfile::PureStrategyProfile(const Game &p_nfg)
  : m_index(1L), m_nfg(p_nfg), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
    m_index += m_profile[pl]->m_offset;
  }
}

//------------------------------------------------------------------------
//            PureStrategyProfile: Data access and manipulation
//------------------------------------------------------------------------

void PureStrategyProfile::SetStrategy(const GameStrategy &s)
{
  m_index += s->m_offset - m_profile[s->GetPlayer()->GetNumber()]->m_offset;
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

GameOutcome PureStrategyProfile::GetOutcome(void) const
{ 
  if (m_nfg->IsTree()) {
    throw UndefinedException();
  }
  else {
    return m_nfg->m_results[m_index]; 
  }
}

void PureStrategyProfile::SetOutcome(GameOutcome p_outcome)
{
  if (m_nfg->IsTree()) {
    throw UndefinedException();
  }
  else {
    m_nfg->m_results[m_index] = p_outcome; 
  }
}

template <class T> T PureStrategyProfile::GetPayoff(int pl) const
{
  if (m_nfg->IsTree()) {
    PureBehavProfile behav(m_nfg);
    for (int i = 1; i <= m_nfg->NumPlayers(); i++) {
      GamePlayer player = m_nfg->GetPlayer(i);
      for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	int act = m_profile[i]->m_behav[iset];
	if (act) {
	  behav.SetAction(player->GetInfoset(iset)->GetAction(act));
	}
      }
    }
    return behav.GetPayoff<T>(pl);
  }
  else {
    GameOutcomeRep *outcome = m_nfg->m_results[m_index];
    if (outcome) {
      return outcome->GetPayoff<T>(pl);
    }
    else {
      return (T) 0;
    }
  }
}

// Explicit instantiations
template double PureStrategyProfile::GetPayoff(int pl) const;
template Rational PureStrategyProfile::GetPayoff(int pl) const;

template<> 
std::string PureStrategyProfile::GetPayoff(int pl) const
{
  if (m_nfg->IsTree()) {
    return lexical_cast<std::string>(GetPayoff<Rational>(pl));
  }
  else {
    GameOutcomeRep *outcome = m_nfg->m_results[m_index]; 
    if (outcome) {
      return outcome->GetPayoff<std::string>(pl);
    }
    else {
      return "0";
    }
  }
}

template <class T>
T PureStrategyProfile::GetStrategyValue(const GameStrategy &p_strategy) const
{
  if (m_nfg->IsTree()) {
    PureStrategyProfile copy(*this);
    copy.SetStrategy(p_strategy);
    return copy.GetPayoff<T>(p_strategy->GetPlayer()->GetNumber());
  }
  else {
    int player = p_strategy->GetPlayer()->GetNumber();
    GameOutcomeRep *outcome = m_nfg->m_results[m_index - m_profile[player]->m_offset + p_strategy->m_offset];
    if (outcome) {
      return outcome->GetPayoff<T>(player);
    }
    else {
      return 0;
    }
  }
}

// Explicit instantiations
template double PureStrategyProfile::GetStrategyValue(const GameStrategy &) const;
template Rational PureStrategyProfile::GetStrategyValue(const GameStrategy &) const;

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

  if (p_node->outcome) {
    payoff += p_node->outcome->GetPayoff<T>(pl);
  }

  if (!p_node->IsTerminal()) {
    if (p_node->GetInfoset()->IsChanceInfoset()) {
      for (int i = 1; i <= p_node->NumChildren(); i++) {
	GameInfosetRep *infoset = p_node->GetInfoset();
	payoff += (infoset->GetActionProb<T>(i) *
		   GetNodeValue<T>(p_node->children[i], pl));
      }
    }
    else {
      int player = p_node->GetPlayer()->GetNumber();
      int iset = p_node->GetInfoset()->GetNumber();
      payoff += GetNodeValue<T>(p_node->children[m_profile[player][iset]->GetNumber()], 
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
//                           class GameRep
//========================================================================

//------------------------------------------------------------------------
//                         GameRep: Lifecycle
//------------------------------------------------------------------------

GameRep::GameRep(void)
  : m_computedValues(false),
    m_chance(new GamePlayerRep(this, 0)), m_root(new GameNodeRep(this, 0))
{ }

/// This convenience function computes the Cartesian product of the
/// elements in dim.
static int Product(const Array<int> &dim)
{
  int accum = 1;
  for (int i = 1; i <= dim.Length(); accum *= dim[i++]);
  return accum;
}
  
GameRep::GameRep(const Array<int> &dim, bool p_sparseOutcomes /* = false */)
  : m_computedValues(true), m_chance(0), m_root(0)
{
  m_results = Array<GameOutcomeRep *>(Product(dim));
  for (int pl = 1; pl <= dim.Length(); pl++)  {
    m_players.Append(new GamePlayerRep(this, pl, dim[pl]));
    m_players[pl]->m_label = lexical_cast<std::string>(pl);
    for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
      m_players[pl]->m_strategies[st]->m_label = lexical_cast<std::string>(st);
    }
  }
  IndexStrategies();

  if (p_sparseOutcomes) {
    for (int cont = 1; cont <= m_results.Length();
	 m_results[cont++] = 0);
  }
  else {
    m_outcomes = Array<GameOutcomeRep *>(m_results.Length());
    for (int i = 1; i <= m_outcomes.Length(); i++) {
      m_outcomes[i] = new GameOutcomeRep(this, i);
    }
    m_results = m_outcomes;
  }
}

GameRep::~GameRep()
{
  if (m_root) m_root->Invalidate();
  if (m_chance) m_chance->Invalidate();

  for (int pl = 1; pl <= m_players.Length(); m_players[pl++]->Invalidate());
  for (int outc = 1; outc <= m_outcomes.Length(); 
       m_outcomes[outc++]->Invalidate());
}

///
/// Creates a copy of the game as a new, separate GameRep object.
/// 
/// NOTE: For now, this takes the "easy" way of writing the game out
/// in text representation, and reading it back in.  For most applications,
/// this would be adequate performance.  However, it might be nice someday
/// to directly do the copying without going through the text translation.
///
Game GameRep::Copy(void) const
{
  std::ostringstream os;

  if (m_root) {
    WriteEfgFile(os);
  }
  else {
    WriteNfgFile(os);
  }

  std::istringstream is(os.str());
  return ReadGame(is);
}

//------------------------------------------------------------------------
//                     GameRep: General data access
//------------------------------------------------------------------------

namespace {

class NotZeroSumException : public Exception {
public:
  virtual ~NotZeroSumException() throw() { }
  const char *what(void) const throw() { return "Game is not constant sum"; }
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
      sum += p_node->GetOutcome()->GetPayoff<Rational>(pl);
    }
  }
  return sum;
}

}

bool GameRep::IsConstSum(void) const
{
  if (m_root) {
    try {
      SubtreeSum(m_root);
      return true;
    }
    catch (NotZeroSumException) {
      return false;
    }
  }
  else {
    PureStrategyProfile profile(const_cast<GameRep *>(this));

    Rational sum(0);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      sum += profile.GetPayoff<Rational>(pl);
    }

    for (StrategyIterator iter(StrategySupport(const_cast<GameRep *>(this)));
	 !iter.AtEnd(); iter++) {
      Rational newsum(0);
      for (int pl = 1; pl <= m_players.Length(); pl++) {
	newsum += iter->GetPayoff<Rational>(pl);
      }

      if (newsum != sum) {
	return false;
      }
    }

    return true;
  }
}

Rational GameRep::GetMinPayoff(int player) const
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

Rational GameRep::GetMaxPayoff(int player) const
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

bool GameRep::IsPerfectRecall(GameInfoset &s1, GameInfoset &s2) const
{
  if (!IsTree()) return true;

  for (int pl = 1; pl <= m_players.Length(); pl++)   {
    GamePlayerRep *player = m_players[pl];
    
    for (int i = 1; i <= player->NumInfosets(); i++)  {
      GameInfosetRep *iset1 = player->GetInfoset(i);
      for (int j = 1; j <= player->NumInfosets(); j++)   {
	GameInfosetRep *iset2 = player->GetInfoset(j);

	bool precedes = false;
	int action = 0;
	
	for (int m = 1; m <= iset2->NumMembers(); m++)  {
	  int n;
	  for (n = 1; n <= iset1->NumMembers(); n++)  {
	    if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)) &&
	        iset1->GetMember(n) != iset2->GetMember(m))  {
	      precedes = true;
	      for (int act = 1; act <= iset1->NumActions(); act++)  {
		if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)->GetChild(act))) {
		  if (action != 0 && action != act)  {
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
	  
	  if (i == j && precedes)  {
	    s1 = iset1;
	    s2 = iset2;
	    return false;
	  }

	  if (n > iset1->NumMembers() && precedes)  {
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
//                  GameRep: Managing the representation
//------------------------------------------------------------------------

void GameRep::NumberNodes(GameNodeRep *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

void GameRep::Canonicalize(void)
{
  if (!m_root)  return;

  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);

  for (int pl = 0; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = (pl) ? m_players[pl] : m_chance;
    
    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      GameInfosetRep *infoset = player->m_infosets[iset];
      for (int i = 1; i < infoset->m_members.Length(); i++) {
	for (int j = 1; j < infoset->m_members.Length() - i; j++) {
	  if (infoset->m_members[j+1]->number < infoset->m_members[j]->number) {
	    GameNodeRep *tmp = infoset->m_members[j];
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
	  GameInfosetRep *tmp = player->m_infosets[j];
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

void GameRep::ClearComputedValues(void) const
{
  if (!m_root)  return;

  for (int pl = 1; pl <= m_players.Length(); pl++) {
    while (m_players[pl]->m_strategies.Length() > 0) {
      m_players[pl]->m_strategies.Remove(1)->Invalidate();
    }
  }

  m_computedValues = false;
}

void GameRep::BuildComputedValues(void)
{
  if (m_computedValues) return;

  Canonicalize();

  if (!IsTree()) return;

  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl]->MakeReducedStrats(m_root, 0);
  }

  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); 
	 m_players[pl]->m_strategies[st++]->m_id = id++);
  }

  m_computedValues = true;
}

//------------------------------------------------------------------------
//                     GameRep: Writing data files
//------------------------------------------------------------------------

static std::string EscapeQuotes(const std::string &s)
{
  std::string ret;
  
  for (unsigned int i = 0; i < s.length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}

static void PrintActions(std::ostream &p_stream, GameInfosetRep *p_infoset)
{ 
  p_stream << "{ ";
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    p_stream << '"' << EscapeQuotes(p_infoset->GetAction(act)->GetLabel()) << "\" ";
    if (p_infoset->IsChanceInfoset()) {
      p_stream << p_infoset->GetActionProb<std::string>(act) << ' ';
    }
  }
  p_stream << "}";
}

static void WriteEfgFile(std::ostream &f, GameNodeRep *n)
{
  if (n->NumChildren() == 0)   {
    f << "t \"" << EscapeQuotes(n->GetLabel()) << "\" ";
    if (n->GetOutcome())  {
      f << n->GetOutcome()->GetNumber() << " \"" <<
	EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
	f << n->GetOutcome()->GetPayoff<std::string>(pl);

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
  PrintActions(f, n->GetInfoset());
  f << " ";
  if (n->GetOutcome())  {
    f << n->GetOutcome()->GetNumber() << " \"" <<
      EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
    f << "{ ";
    for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
      f << n->GetOutcome()->GetPayoff<std::string>(pl);
      
      if (pl < n->GetGame()->NumPlayers())
	f << ", ";
      else
	f << " }\n";
    }
  }
  else
    f << "0\n";

  for (int i = 1; i <= n->NumChildren(); WriteEfgFile(f, n->GetChild(i++)));
}

void GameRep::WriteEfgFile(std::ostream &p_file) const
{
  if (!IsTree())  throw UndefinedException();

  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, m_root);
}

void GameRep::WriteEfgFile(std::ostream &p_file, const GameNode &p_root) const
{
  if (!IsTree())  throw UndefinedException();

  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, p_root);
}

void GameRep::WriteNfgFile(std::ostream &p_file) const
{ 
  if (IsTree()) {
    // FIXME: Building computed values is logically const.
    const_cast<GameRep *>(this)->BuildComputedValues();
  }

  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++)
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    GamePlayerRep *player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetLabel()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  if (IsTree()) {
    // For trees, we write the payoff version, since there need not be
    // a one-to-one correspondence between outcomes and entries, when there
    // are chance moves.
    StrategyIterator iter(Game(const_cast<GameRep *>(this)));
    
    for (; !iter.AtEnd(); iter++) {
      for (int pl = 1; pl <= NumPlayers(); pl++) {
	p_file << iter->GetPayoff<Rational>(pl) << " ";
      }
      p_file << "\n";
    }
    
  }
  else {
    int ncont = 1;
    for (int i = 1; i <= NumPlayers(); i++) {
      ncont *= m_players[i]->m_strategies.Length();
    }

    p_file << "{\n";
    for (int outc = 1; outc <= m_outcomes.Length(); outc++)   {
      p_file << "{ \"" << EscapeQuotes(m_outcomes[outc]->m_label) << "\" ";
      for (int pl = 1; pl <= m_players.Length(); pl++)  {
	p_file << (const std::string &) m_outcomes[outc]->m_payoffs[pl];

	if (pl < m_players.Length()) {
	  p_file << ", ";
	}
	else {
	  p_file << " }\n";
	}
      }
    }
    p_file << "}\n";
  
    for (int cont = 1; cont <= ncont; cont++)  {
      if (m_results[cont] != 0) {
	p_file << m_results[cont]->m_number << ' ';
      }
      else {
	p_file << "0 ";
      }
    }
  }

  p_file << '\n';
}

//------------------------------------------------------------------------
//                   GameRep: Dimensions of the game
//------------------------------------------------------------------------

PVector<int> GameRep::NumActions(void) const
{
  if (!IsTree()) throw UndefinedException();

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

PVector<int> GameRep::NumMembers(void) const
{
  if (!IsTree()) throw UndefinedException();

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

Array<int> GameRep::NumStrategies(void) const
{
  Array<int> dim(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    dim[pl] = m_players[pl]->m_strategies.Length();
  }
  return dim;
}

GameStrategy GameRep::GetStrategy(int p_index) const
{
  for (int pl = 1, i = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); st++, i++) {
      if (p_index == i) {
	return m_players[pl]->m_strategies[st];
      }
    }
  }
  throw IndexException();
}

int GameRep::BehavProfileLength(void) const
{
  if (!IsTree()) throw UndefinedException();

  int sum = 0;

  for (int i = 1; i <= m_players.Length(); i++)
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++)
      sum += m_players[i]->m_infosets[j]->m_actions.Length();

  return sum;
}

int GameRep::MixedProfileLength(void) const
{
  int strats = 0;
  for (int i = 1; i <= m_players.Length();
       strats += m_players[i++]->m_strategies.Length());
  return strats;
}


//------------------------------------------------------------------------
//                         GameRep: Players
//------------------------------------------------------------------------

GamePlayer GameRep::NewPlayer(void)
{
  if (m_root) {
    GamePlayerRep *player = new GamePlayerRep(this, m_players.Length() + 1);
    m_players.Append(player);
    
    for (int outc = 1; outc <= m_outcomes.Last(); outc++) {
      m_outcomes[outc]->m_payoffs.Append(Number());
    }

    ClearComputedValues();
    return player;
  }
  else {
    GamePlayerRep *player = new GamePlayerRep(this, m_players.Length() + 1, 1);
    m_players.Append(player);

    for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
      m_outcomes[outc]->m_payoffs.Append(Number());
    }

    return player;
  }
}

//------------------------------------------------------------------------
//                     GameRep: Information sets
//------------------------------------------------------------------------

GameInfoset GameRep::GetInfoset(int p_index) const
{
  if (!IsTree()) throw UndefinedException();

  int index = 1;
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = m_players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (index++ == p_index) {
	return player->GetInfoset(iset);
      }
    }
  }
  throw IndexException();
}

Array<int> GameRep::NumInfosets(void) const
{
  if (!IsTree()) throw UndefinedException();

  Array<int> foo(m_players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }

  return foo;
}

GameAction GameRep::GetAction(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = m_players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfosetRep *infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (index++ == p_index) {
	  return infoset->GetAction(act);
	}
      }
    }
  }
  throw IndexException();
}

//------------------------------------------------------------------------
//                        GameRep: Outcomes
//------------------------------------------------------------------------

GameOutcome GameRep::NewOutcome(void)
{
  m_outcomes.Append(new GameOutcomeRep(this, m_outcomes.Length() + 1));
  return m_outcomes[m_outcomes.Last()];
}

void GameRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  if (m_root) {
    m_root->DeleteOutcome(p_outcome);
  }
  else {
    for (int i = 1; i <= m_results.Length(); i++) {
      if (m_results[i] == p_outcome) {
	m_results[i] = 0;
      }
    }
  }

  m_outcomes.Remove(m_outcomes.Find(p_outcome))->Invalidate();

  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_number = outc;
  }
  ClearComputedValues();
}


//------------------------------------------------------------------------
//                          GameRep: Nodes
//------------------------------------------------------------------------

static int CountNodes(GameNodeRep *p_node)
{
  int num = 1;
  for (int i = 1; i <= p_node->NumChildren(); 
       num += CountNodes(p_node->GetChild(i++)));
  return num;
}

int GameRep::NumNodes(void) const
{
  return CountNodes(m_root);
}

//------------------------------------------------------------------------
//                  GameRep: Private auxiliary functions
//------------------------------------------------------------------------

void GameRep::IndexStrategies(void)
{
  long offset = 1L;

  for (int pl = 1; pl <= m_players.Length(); pl++)  {
    for (GameStrategyIterator strategy(m_players[pl]->m_strategies);
	 !strategy.AtEnd(); strategy++) {
      strategy->m_number = strategy.GetIndex();
      strategy->m_offset = (strategy.GetIndex() - 1) * offset;
    }
    offset *= m_players[pl]->NumStrategies();
  }

  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); 
	 m_players[pl]->m_strategies[st++]->m_id = id++);
  }
}

/// This rebuilds a new table of outcomes after the game has been
/// redimensioned (change in the number of strategies).  Strategies
/// numbered -1 are identified as the new strategies.
void GameRep::RebuildTable(void)
{
  long size = 1L;
  Array<long> offsets(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    offsets[pl] = size;
    size *= m_players[pl]->NumStrategies();
  }

  Array<GameOutcomeRep *> newResults(size);
  for (int i = 1; i <= newResults.Length(); newResults[i++] = 0);

  for (StrategyIterator iter(StrategySupport(const_cast<GameRep *>(this)));
       !iter.AtEnd(); iter++) {
    long newindex = 1L;
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      if (iter.m_profile.GetStrategy(pl)->m_offset < 0) {
	// This is a contingency involving a new strategy... skip
	newindex = -1L;
	break;
      }
      else {
	newindex += (iter.m_profile.GetStrategy(pl)->m_number - 1) * offsets[pl];
      }
    }

    if (newindex >= 1) {
      newResults[newindex] = m_results[iter.m_profile.m_index];
    }
  }

  m_results = newResults;

  IndexStrategies();
}



}  // end namespace Gambit
