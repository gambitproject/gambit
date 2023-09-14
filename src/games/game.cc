//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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
#include <numeric>

#include "gambit.h"
#include "gametree.h"
#include "gametable.h"

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number)
  : m_game(p_game), m_number(p_number),
    m_payoffs(m_game->NumPlayers()), m_unrestricted(nullptr)
{ }


//========================================================================
//                      class GameStrategyRep
//========================================================================

void GameStrategyRep::DeleteStrategy()
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
  : m_game(p_game), m_number(p_id), m_strategies(p_strats), m_unrestricted(nullptr)
{ 
  for (int j = 1; j <= p_strats; j++) {
    m_strategies[j] = new GameStrategyRep(this);
    m_strategies[j]->m_number = j;
  }
}

GamePlayerRep::~GamePlayerRep()
{
  for (auto infoset : m_infosets) {
    infoset->Invalidate();
  }
  for (auto strategy : m_strategies) {
    strategy->Invalidate();
  }
}


GameStrategy GamePlayerRep::NewStrategy()
{
  if (m_game->IsTree())  throw UndefinedException();

  auto *strategy = new GameStrategyRep(this);
  m_strategies.push_back(strategy);
  strategy->m_number = m_strategies.size();
  strategy->m_offset = -1;   // this flags this action as new
  dynamic_cast<GameTableRep *>(m_game)->RebuildTable();
  return strategy;
}

void GamePlayerRep::MakeStrategy()
{
  Array<int> c(NumInfosets());
  
  for (int i = 1; i <= NumInfosets(); i++)  {
    if (m_infosets[i]->flag == 1)
      c[i] = m_infosets[i]->whichbranch;
    else
      c[i] = 0;
  }
  
  auto *strategy = new GameStrategyRep(this);
  m_strategies.push_back(strategy);
  strategy->m_number = m_strategies.Length();
  strategy->m_behav = c;
  strategy->m_label = "";

  // We generate a default labeling -- probably should be changed in future
  if (!strategy->m_behav.empty()) {
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

  if (!n->GetParent())  n->ptr = nullptr;

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
      n->ptr = nullptr;
      if (nn != nullptr)
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
	m = nullptr;
      }
      else {
	m = dynamic_cast<GameTreeNodeRep *>(nn->GetNextSibling().operator->());
      }
      if (m || nn->m_parent->ptr == nullptr)   break;
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
//                    class PureStrategyProfileRep
//========================================================================

PureStrategyProfileRep::PureStrategyProfileRep(const Game &p_game) 
  : m_nfg(p_game), m_profile(p_game->NumPlayers())
{
  for (size_t pl = 1; pl <= m_nfg->NumPlayers(); pl++)  {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
  }
}

bool PureStrategyProfileRep::IsNash() const
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    GamePlayer player = m_nfg->GetPlayer(pl);
    Rational current = GetPayoff(player);
    for (GameStrategyArray::const_iterator strategy = player->Strategies().begin();
	 strategy != player->Strategies().end();
	 ++strategy)  {
      if (GetStrategyValue(*strategy) > current) {
        return false;
      }
    }
  }
  return true;
}

bool PureStrategyProfileRep::IsStrictNash() const
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    GamePlayer player = m_nfg->GetPlayer(pl);
    Rational current = GetPayoff(player);
    for (GameStrategyArray::const_iterator strategy = player->Strategies().begin();
	 strategy != player->Strategies().end();
	 ++strategy) {
      if (GetStrategyValue(*strategy) >= current) {
        return false;
      }
    }
  }
  return true;
}

bool PureStrategyProfileRep::IsBestResponse(const GamePlayer &p_player) const
{
  Rational current = GetPayoff(p_player);
  for (GameStrategyArray::const_iterator strategy = p_player->Strategies().begin();
       strategy != p_player->Strategies().end();
       ++strategy) {
    if (GetStrategyValue(*strategy) > current) {
      return false;
    }
  }
  return true;
}

List<GameStrategy> 
PureStrategyProfileRep::GetBestResponse(const GamePlayer &p_player) const
{
  GameStrategyArray::const_iterator strategy = p_player->Strategies().begin();
  Rational max_payoff = GetStrategyValue(*strategy);
  List<GameStrategy> br;
  br.push_back(*strategy);
  for (++strategy; strategy != p_player->Strategies().end(); ++strategy)  {
    Rational this_payoff = GetStrategyValue(*strategy);
    if (this_payoff > max_payoff) {
      br.clear();
      max_payoff = this_payoff;
    }
    if (this_payoff >= max_payoff) {
      br.push_back(*strategy);
    }
  }
  return br;
}

MixedStrategyProfile<Rational>
PureStrategyProfileRep::ToMixedStrategyProfile() const
{
  MixedStrategyProfile<Rational> temp(m_nfg->NewMixedStrategyProfile(Rational(0)));
  static_cast<Vector<Rational> &>(temp).operator=(Rational(0));
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    temp[GetStrategy(m_nfg->GetPlayer(pl))] = Rational(1);
  }
  return temp;
}

PureStrategyProfile PureStrategyProfileRep::Unrestrict() const
{
  PureStrategyProfile u = m_nfg->Unrestrict()->NewPureStrategyProfile();
  for (auto player : this->m_nfg->GetPlayers()) {
    u->SetStrategy(GetStrategy(player)->Unrestrict());
  }
  return u;
}

//========================================================================
//                       class PureBehaviorProfile
//========================================================================

//------------------------------------------------------------------------
//                     PureBehaviorProfile: Lifecycle
//------------------------------------------------------------------------

PureBehaviorProfile::PureBehaviorProfile(Game p_efg)
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
//              PureBehaviorProfile: Data access and manipulation
//------------------------------------------------------------------------

GameAction PureBehaviorProfile::GetAction(const GameInfoset &infoset) const
{
  return m_profile[infoset->GetPlayer()->GetNumber()][infoset->GetNumber()];
}

void PureBehaviorProfile::SetAction(const GameAction &action)
{
  m_profile[action->GetInfoset()->GetPlayer()->GetNumber()]
    [action->GetInfoset()->GetNumber()] = action;
}

template <class T> 
T PureBehaviorProfile::GetPayoff(const GameNode &p_node,
				 int pl) const
{
  T payoff(0);

  auto *node = dynamic_cast<GameTreeNodeRep *>(p_node.operator->());

  if (node->outcome) {
    payoff += static_cast<T>(node->outcome->GetPayoff(pl));
  }

  if (!node->IsTerminal()) {
    if (node->GetInfoset()->IsChanceInfoset()) {
      for (int i = 1; i <= node->NumChildren(); i++) {
        GameTreeInfosetRep *infoset = node->infoset;
        payoff += (static_cast<T>(infoset->GetActionProb(i)) *
                   GetPayoff<T>(node->children[i], pl));
      }
    } else {
      int player = node->GetPlayer()->GetNumber();
      int iset = node->GetInfoset()->GetNumber();
      payoff += GetPayoff<T>(node->children[m_profile[player][iset]->GetNumber()],
                             pl);
    }
  }

  return payoff;
}

// Explicit instantiations
template double PureBehaviorProfile::GetPayoff(const GameNode &, int pl) const;
template Rational PureBehaviorProfile::GetPayoff(const GameNode &, int pl) const;

template <class T>
T PureBehaviorProfile::GetPayoff(const GameAction &p_action) const
{
  PureBehaviorProfile copy(*this);
  copy.SetAction(p_action);
  return copy.GetPayoff<T>(p_action->GetInfoset()->GetPlayer()->GetNumber());
}

// Explicit instantiations
template double PureBehaviorProfile::GetPayoff(const GameAction &) const;
template Rational PureBehaviorProfile::GetPayoff(const GameAction &) const;

bool PureBehaviorProfile::IsAgentNash() const
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    GamePlayer player = m_efg->GetPlayer(pl);
    Rational current = GetPayoff<Rational>(player);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	GameAction action = infoset->GetAction(act);
	if (GetPayoff<Rational>(action) > current)  {
	  return false;
	}
      }
    }
  }
  return true;
}

MixedBehaviorProfile<Rational>
PureBehaviorProfile::ToMixedBehaviorProfile() const
{
  MixedBehaviorProfile<Rational> temp(m_efg);
  static_cast<Vector<Rational> &>(temp) = Rational(0);
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    GamePlayer player = m_efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      temp(GetAction(player->GetInfoset(iset))) = Rational(1);
    }
  }
  return temp;
}

//========================================================================
//                            class GameRep
//========================================================================

//------------------------------------------------------------------------
//                     GameRep: Writing data files
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

}  // end anonymous namespace

///
/// Write the game to a savefile in .nfg payoff format.
///
/// This implements writing a game to a .nfg savefile.  As it uses
/// only publicly-accessible operations, it is in principle valid
/// for any game.  It writes the payoff format, as it is not required
/// that a game representation implements an outcome for each possible
/// pure strategy profile.  For example, in extensive games with chance
/// moves, the outcome from a pure strategy profile is not deterministic,
/// but rather a probability distribution over outcomes.
///
void GameRep::WriteNfgFile(std::ostream &p_file) const
{ 
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

  for (StrategyProfileIterator iter(StrategySupportProfile(Game(const_cast<GameRep *>(this))));
       !iter.AtEnd(); iter++) {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      p_file << (*iter)->GetPayoff(pl) << " ";
    }
    p_file << "\n";
  }
  p_file << '\n';
}


//========================================================================
//                       class GameExplicitRep
//========================================================================


//------------------------------------------------------------------------
//                     GameExplicitRep: Lifecycle
//------------------------------------------------------------------------

GameExplicitRep::~GameExplicitRep()
{
  for (auto player : m_players) {
    player->Invalidate();
  }
  for (auto outcome : m_outcomes) {
    outcome->Invalidate();
  }
}

//------------------------------------------------------------------------
//                  GameExplicitRep: General data access
//------------------------------------------------------------------------

Rational GameExplicitRep::GetMinPayoff(int player) const
{
  int p1, p2;
  
  if (m_outcomes.empty()) {
    return Rational(0);
  }

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }

  Rational minpay = static_cast<Rational>(m_outcomes.front()->GetPayoff(p1));
  for (auto outcome : m_outcomes) {
    for (int p = p1; p <= p2; p++) {
      minpay = std::min(minpay, static_cast<Rational>(outcome->GetPayoff(p)));
    }
  }
  return minpay;
}

Rational GameExplicitRep::GetMaxPayoff(int player) const
{
  int p1, p2;

  if (m_outcomes.empty()) {
    return Rational(0);
  }

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }

  Rational maxpay = static_cast<Rational>(m_outcomes.front()->GetPayoff(p1));
  for (auto outcome : m_outcomes) {
    for (int p = p1; p <= p2; p++) {
      maxpay = std::max(maxpay, static_cast<Rational>(outcome->GetPayoff(p)));
    }
  }
  return maxpay;
}

//------------------------------------------------------------------------
//                GameExplicitRep: Dimensions of the game
//------------------------------------------------------------------------

Array<int> GameExplicitRep::NumStrategies() const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  Array<int> dim(m_players.size());
  for (int pl = 1; pl <= m_players.size(); pl++) {
    dim[pl] = m_players[pl]->m_strategies.size();
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

int GameExplicitRep::NumStrategyContingencies() const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  return std::accumulate(
          m_players.begin(), m_players.end(), 1,
          [](int ncont, GamePlayerRep *p) { return ncont * p->m_strategies.size(); }
  );
}

int GameExplicitRep::MixedProfileLength() const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  return std::accumulate(
          m_players.begin(), m_players.end(), 0,
          [](int size, GamePlayerRep *p) { return size + p->m_strategies.size(); }
  );
}


//------------------------------------------------------------------------
//                      GameExplicitRep: Outcomes
//------------------------------------------------------------------------

GameOutcome GameExplicitRep::NewOutcome()
{
  m_outcomes.push_back(new GameOutcomeRep(this, m_outcomes.size() + 1));
  return m_outcomes.back();
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


