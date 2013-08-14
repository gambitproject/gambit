//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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
T PureBehavProfile::GetPayoff(const GameNode &p_node,
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
		   GetPayoff<T>(node->children[i], pl));
      }
    }
    else {
      int player = node->GetPlayer()->GetNumber();
      int iset = node->GetInfoset()->GetNumber();
      payoff += GetPayoff<T>(node->children[m_profile[player][iset]->GetNumber()], 
				pl);
    }
  }

  return payoff;
}

// Explicit instantiations
template double PureBehavProfile::GetPayoff(const GameNode &, int pl) const;
template Rational PureBehavProfile::GetPayoff(const GameNode &, int pl) const;

template <class T>
T PureBehavProfile::GetPayoff(const GameAction &p_action) const
{
  PureBehavProfile copy(*this);
  copy.SetAction(p_action);
  return copy.GetPayoff<T>(p_action->GetInfoset()->GetPlayer()->GetNumber());
}

// Explicit instantiations
template double PureBehavProfile::GetPayoff(const GameAction &) const;
template Rational PureBehavProfile::GetPayoff(const GameAction &) const;

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
//                  GameExplicitRep: General data access
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

int GameExplicitRep::NumStrategyContingencies(void) const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  int ncont = 1;
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    ncont *= m_players[pl]->m_strategies.Length();
  }
  return ncont;
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

//------------------------------------------------------------------------
//                GameExplicitRep: Writing data files
//------------------------------------------------------------------------

void GameExplicitRep::Write(std::ostream &p_stream,
			    const std::string &p_format /*="native"*/) const
{
  if (p_format == "efg" ||
      (p_format == "native" && IsTree())) {
    WriteEfgFile(p_stream);
  }
  else if (p_format == "nfg" ||
	   (p_format == "native" && !IsTree())) {
    WriteNfgFile(p_stream);
  }
  else {
    throw UndefinedException();
  }
}


}  // end namespace Gambit
