//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gameagg.cc
// Implementation of action-graph game representation
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

#include "gambit.h"
#include "gameagg.h"

namespace Gambit {

//========================================================================
//                  class AggPureStrategyProfileRep
//========================================================================

class AggPureStrategyProfileRep : public PureStrategyProfileRep {
public:
  explicit AggPureStrategyProfileRep(const Game &p_game)
    : PureStrategyProfileRep(p_game) { }
  PureStrategyProfileRep *Copy() const override
  { return new AggPureStrategyProfileRep(*this); }

  void SetStrategy(const GameStrategy &) override;
  GameOutcome GetOutcome() const override { throw UndefinedException(); }
  void SetOutcome(GameOutcome p_outcome) override
  { throw UndefinedException(); }
  Rational GetPayoff(int pl) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//       AggPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void AggPureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Rational AggPureStrategyProfileRep::GetPayoff(int pl) const
{
  agg::AGG *aggPtr = dynamic_cast<GameAggRep &>(*m_nfg).aggPtr;
  std::vector<int> s(aggPtr->getNumPlayers());
  for (int i = 1; i <= aggPtr->getNumPlayers(); i++) {
    s[i-1] = m_profile[i]->GetNumber() -1;
  }
  return Rational(aggPtr->getPurePayoff(pl-1, s));
}

Rational
AggPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  agg::AGG *aggPtr = dynamic_cast<GameAggRep &>(*m_nfg).aggPtr;
  std::vector<int> s(aggPtr->getNumPlayers());
  for (int i = 1; i <= aggPtr->getNumPlayers(); i++) {
    s[i-1] = m_profile[i]->GetNumber() - 1;
  }
  s[player-1] = p_strategy->GetNumber() - 1;
  return Rational(aggPtr->getPurePayoff(player-1, s));
}

//------------------------------------------------------------------------
//                        GameAggRep: Lifecycle
//------------------------------------------------------------------------

GameAggRep::GameAggRep(agg::AGG *p_aggPtr)
  : aggPtr(p_aggPtr)
{
  for (int pl = 1; pl <= aggPtr->getNumPlayers(); pl++) {
    m_players.push_back(new GamePlayerRep(this, pl, aggPtr->getNumActions(pl-1)));
    m_players[pl]->m_label = lexical_cast<std::string>(pl);
    for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
      m_players[pl]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
    }
  }
  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length();
	 m_players[pl]->m_strategies[st++]->m_id = id++);
  }
}

Game GameAggRep::Copy() const
{
  std::ostringstream os;
  WriteAggFile(os);
  std::istringstream is(os.str());
  return ReadAggFile(is);
}

//------------------------------------------------------------------------
//                  GameAggRep: Dimensions of the game
//------------------------------------------------------------------------

Array<int> GameAggRep::NumStrategies() const
{
  Array<int> ns;
  for (int pl = 1; pl <= aggPtr->getNumPlayers(); pl++) {
    ns.push_back(m_players[pl]->NumStrategies());
  }
  return ns;
}

GameStrategy GameAggRep::GetStrategy(int p_index) const
{
  for (int pl = 1; pl <= aggPtr->getNumPlayers(); pl++) {
    if (m_players[pl]->NumStrategies() >= p_index) {
      return m_players[pl]->GetStrategy(p_index);
    }
    else {
      p_index -= m_players[pl]->NumStrategies();
    }
  }
  throw IndexException();
}
 
//------------------------------------------------------------------------
//                    GameAggRep: Factory functions
//------------------------------------------------------------------------

PureStrategyProfile GameAggRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(new AggPureStrategyProfileRep(const_cast<GameAggRep *>(this)));
}

MixedStrategyProfile<double> GameAggRep::NewMixedStrategyProfile(double) const
{
  return MixedStrategyProfile<double>(
    new AggMixedStrategyProfileRep<double>(StrategySupportProfile(const_cast<GameAggRep *>(this)))
  );
}

MixedStrategyProfile<Rational> GameAggRep::NewMixedStrategyProfile(const Rational &) const
{
  return MixedStrategyProfile<Rational>(
    new AggMixedStrategyProfileRep<Rational>(StrategySupportProfile(const_cast<GameAggRep *>(this)))
  );
}
MixedStrategyProfile<double> GameAggRep::NewMixedStrategyProfile(double, const StrategySupportProfile& spt) const
{
  return MixedStrategyProfile<double>(new AggMixedStrategyProfileRep<double>(spt));
}

MixedStrategyProfile<Rational> GameAggRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile& spt) const
{
  return MixedStrategyProfile<Rational>(new AggMixedStrategyProfileRep<Rational>(spt));
}

//------------------------------------------------------------------------
//                  GameAggRep: General data access
//------------------------------------------------------------------------

bool GameAggRep::IsConstSum() const
{
  AggPureStrategyProfileRep profile(const_cast<GameAggRep *>(this));

  Rational sum(0);
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    sum += profile.GetPayoff(pl);
  }

  for (StrategyProfileIterator iter(StrategySupportProfile(const_cast<GameAggRep *>(this)));
       !iter.AtEnd(); iter++) {
    Rational newsum(0);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      newsum += (*iter)->GetPayoff(pl);
    }
    if (newsum != sum) {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------
//                   GameAggRep: Writing data files
//------------------------------------------------------------------------

void GameAggRep::Write(std::ostream &p_stream,
		       const std::string &p_format /*="native"*/) const
{
  if (p_format == "native" || p_format == "agg") {
    WriteAggFile(p_stream);
  }
  else if (p_format == "nfg") {
    WriteNfgFile(p_stream);
  }
  else {
    throw UndefinedException();
  }
}

void GameAggRep::WriteAggFile(std::ostream &s) const
{
  s << "#AGG" << std::endl;
  s << aggPtr->getNumPlayers() << std::endl;
  s << aggPtr->getNumActionNodes() << std::endl;
  s << aggPtr->getNumFunctionNodes() << std::endl;
  for (int i = 0; i < aggPtr->getNumPlayers(); i++) {
    s << aggPtr->getNumActions(i) << " ";
  }
  s << std::endl;
  for (int i = 0; i < aggPtr->getNumPlayers(); i++) {
    std::copy(aggPtr->getActionSet(i).begin(), 
	      aggPtr->getActionSet(i).end(),
	      std::ostream_iterator<int>(s," "));
    s << std::endl;
  }
  aggPtr->printActionGraph(s);
  s << std::endl;
  // types of function nodes
  aggPtr->printTypes(s);
  s << std::endl;

  // payoffs
  for (int i = 0; i < aggPtr->getNumActionNodes(); i++) {
    s << "1" << std::endl; //type of payoff output
    aggPtr->printPayoffs(s, i);
    s << std::endl;
  }
}

Game GameAggRep::ReadAggFile(std::istream& in){
  agg::AGG *aggPtr = agg::AGG::makeAGG(in);
  if (!aggPtr) {
    throw InvalidFileException();
  }
  return new GameAggRep(aggPtr);
}

}  // end namespace Gambit
