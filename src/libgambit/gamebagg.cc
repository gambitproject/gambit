//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gamebagg.cc
// Implementation of Bayesian action-graph game representation
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
#include "gamebagg.h"

namespace Gambit {

//========================================================================
//                  class BagentPureStrategyProfileRep
//========================================================================

class BagentPureStrategyProfileRep : public PureStrategyProfileRep {
public:
  BagentPureStrategyProfileRep(const Game &p_game)
    : PureStrategyProfileRep(p_game) { }
  virtual PureStrategyProfileRep *Copy(void) const
  {  return new BagentPureStrategyProfileRep(*this); }
  virtual void SetStrategy(const GameStrategy &);
  virtual GameOutcome GetOutcome(void) const
  { throw UndefinedException(); }
  virtual void SetOutcome(GameOutcome p_outcome)
  { throw UndefinedException(); }
  virtual Rational GetPayoff(int pl) const;
  virtual Rational GetStrategyValue(const GameStrategy &) const;
};

//------------------------------------------------------------------------
//       BagentPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void BagentPureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Rational BagentPureStrategyProfileRep::GetPayoff(int pl) const
{
  bagg *baggPtr = dynamic_cast<GameBagentRep &>(*m_nfg).baggPtr;
  int s[m_nfg->NumPlayers()];
  for (int i = 1; i <= m_nfg->NumPlayers(); i++) {
    s[i-1] = m_profile[i]->GetNumber() - 1;
  }
  int bp = dynamic_cast<GameBagentRep &>(*m_nfg).agent2baggPlayer[pl];
  int tp = pl - 1 - baggPtr->typeOffset[bp-1];
  return baggPtr->getPurePayoff(bp-1,tp,s);
}

Rational
BagentPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  bagg *baggPtr = dynamic_cast<GameBagentRep &>(*m_nfg).baggPtr;
  int s[m_nfg->NumPlayers()];
  for (int i= 1; i <= m_nfg->NumPlayers(); i++) {
    s[i-1] = m_profile[i]->GetNumber() - 1;
  }
  s[player-1] = p_strategy->GetNumber() - 1;
  int bp = dynamic_cast<GameBagentRep &>(*m_nfg).agent2baggPlayer[player];
  int tp = player - 1 - baggPtr->typeOffset[bp-1];
  return baggPtr->getPurePayoff(bp-1,tp,s);
}


//------------------------------------------------------------------------
//                      GameBagentRep: Lifecycle
//------------------------------------------------------------------------

GameBagentRep::GameBagentRep(bagg* _baggPtr) 
  : baggPtr(_baggPtr), agent2baggPlayer(_baggPtr->getNumTypes())
{
  int k = 1;
  for (int pl = 1; pl <= baggPtr->getNumPlayers(); pl++) {
    for (int j = 0; j < baggPtr->getNumTypes(pl-1); j++,k++) {
      m_players.Append(new GamePlayerRep(this, k,
					 baggPtr->getNumActions(pl-1, j)));
      m_players[k]->m_label = lexical_cast<std::string>(k);
      agent2baggPlayer[k] = pl;
      for (int st = 1; st <= m_players[k]->NumStrategies(); st++) {
	m_players[k]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
      }
    }
  }
  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length();
	 m_players[pl]->m_strategies[st++]->m_id = id++);
  }
}

Game GameBagentRep::Copy(void) const
{
  std::ostringstream os;
  WriteBaggFile(os);
  std::istringstream is(os.str());
  return ReadBaggFile(is);
}

//------------------------------------------------------------------------
//                 GameBagentRep: Dimensions of the game
//------------------------------------------------------------------------

Array<int> GameBagentRep::NumStrategies(void) const
{
  Array<int> ns;
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    ns.Append(m_players[pl]->m_strategies.Length());
  }
  return ns;
}

int GameBagentRep::MixedProfileLength(void) const 
{
  int res = 0;
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    res += m_players[pl]->m_strategies.Length();
  }
  return res;
}


PureStrategyProfile GameBagentRep::NewPureStrategyProfile(void) const
{
  return PureStrategyProfile(new BagentPureStrategyProfileRep(const_cast<GameBagentRep *>(this)));
}

MixedStrategyProfile<double> GameBagentRep::NewMixedStrategyProfile(double) const
{
  return new BagentMixedStrategyProfileRep<double>(StrategySupportProfile(const_cast<GameBagentRep *>(this)));
}

MixedStrategyProfile<Rational> GameBagentRep::NewMixedStrategyProfile(const Rational &) const
{
  return new BagentMixedStrategyProfileRep<Rational>(StrategySupportProfile(const_cast<GameBagentRep *>(this)));
}
MixedStrategyProfile<double> GameBagentRep::NewMixedStrategyProfile(double, const StrategySupportProfile& spt) const
{
  return new BagentMixedStrategyProfileRep<double>(spt);
}

MixedStrategyProfile<Rational> GameBagentRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile& spt) const
{
  return new BagentMixedStrategyProfileRep<Rational>(spt);
}

//------------------------------------------------------------------------
//                   GameBagentRep: Writing data files
//------------------------------------------------------------------------

void GameBagentRep::Write(std::ostream &p_stream,
			  const std::string &p_format /*="native"*/) const
{
  if (p_format == "native" || p_format == "bagg") {
    WriteBaggFile(p_stream);
  }
  else if (p_format == "nfg") {
    WriteNfgFile(p_stream);
  }
  else {
    throw UndefinedException();
  }
}

void GameBagentRep::WriteBaggFile(std::ostream &s) const
{
  s << (*baggPtr);
}

Game GameBagentRep::ReadBaggFile(std::istream &in)
{
  bagg *baggPtr = bagg::makeBAGG(in);
  if (!baggPtr) {
    throw InvalidFileException();
  }
  return new GameBagentRep(baggPtr);
}

}  // end namespace Gambit

