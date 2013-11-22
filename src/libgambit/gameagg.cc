//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
#include <sstream>

#include "libgambit.h"
#include "gameagg.h"

namespace Gambit {

//========================================================================
//                  class AggPureStrategyProfileRep
//========================================================================

class AggPureStrategyProfileRep : public PureStrategyProfileRep {


public:
  AggPureStrategyProfileRep(const Game &p_game);
  virtual PureStrategyProfileRep *Copy(void) const;
  //virtual long GetIndex(void) const { return m_index; }
  virtual void SetStrategy(const GameStrategy &);
  virtual GameOutcome GetOutcome(void) const;
  virtual void SetOutcome(GameOutcome p_outcome);
  virtual Rational GetPayoff(int pl) const;
  virtual Rational GetStrategyValue(const GameStrategy &) const;
};

//------------------------------------------------------------------------
//               AggPureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

AggPureStrategyProfileRep::AggPureStrategyProfileRep(const Game &p_game)
{

  m_nfg = p_game;
  m_profile = Array<GameStrategy>(p_game->NumPlayers());
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++)   {
    m_profile[pl] = p_game->GetPlayer(pl)->GetStrategy(1);
  }
}

PureStrategyProfileRep *AggPureStrategyProfileRep::Copy(void) const
{
  return new AggPureStrategyProfileRep(*this);
}
//------------------------------------------------------------------------
//       AggPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void AggPureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  //m_index += s->m_offset - m_profile[s->GetPlayer()->GetNumber()]->m_offset;
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

GameOutcome AggPureStrategyProfileRep::GetOutcome(void) const
{
	throw UndefinedException();
}

void AggPureStrategyProfileRep::SetOutcome(GameOutcome p_outcome)
{
	throw UndefinedException();
}

Rational AggPureStrategyProfileRep::GetPayoff(int pl) const
{
	agg* aggPtr = dynamic_cast<GameAggRep &>(*m_nfg).aggPtr;
	int s[aggPtr->getNumPlayers()];
	for(int p=1; p<=aggPtr->getNumPlayers(); ++p){
		s[p-1] = m_profile[p]->GetNumber() -1;
	}
	return aggPtr->getPurePayoff(pl-1,s);
}

Rational
AggPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  agg* aggPtr = dynamic_cast<GameAggRep &>(*m_nfg).aggPtr;
  int s[aggPtr->getNumPlayers()];
  for(int p=1; p<=aggPtr->getNumPlayers(); ++p){
  		s[p-1] = m_profile[p]->GetNumber() -1;
  }
  s[player-1] = p_strategy->GetNumber() -1;
  return aggPtr->getPurePayoff(player-1,s);
}







//------------------------------------------------------------------------
//                   GameAGGRep: Factory functions
//------------------------------------------------------------------------

PureStrategyProfile GameAggRep::NewPureStrategyProfile(void) const
{
    return PureStrategyProfile(new AggPureStrategyProfileRep(const_cast<GameAggRep *>(this)));
}

MixedStrategyProfile<double> GameAggRep::NewMixedStrategyProfile(double) const
{
  return new AggMixedStrategyProfileRep<double>(StrategySupport(const_cast<GameAggRep *>(this)));
}

MixedStrategyProfile<Rational> GameAggRep::NewMixedStrategyProfile(const Rational &) const
{
  return new AggMixedStrategyProfileRep<Rational>(StrategySupport(const_cast<GameAggRep *>(this)));
}
MixedStrategyProfile<double> GameAggRep::NewMixedStrategyProfile(double, const StrategySupport& spt) const
{
  return new AggMixedStrategyProfileRep<double>(spt);
}

MixedStrategyProfile<Rational> GameAggRep::NewMixedStrategyProfile(const Rational &, const StrategySupport& spt) const
{
  return new AggMixedStrategyProfileRep<Rational>(spt);
}

//------------------------------------------------------------------------
//                   GameAGGRep: Lifecycle
//------------------------------------------------------------------------
Game GameAggRep::Copy(void) const
{
  std::ostringstream os;
  WriteAggFile(os);
  std::istringstream is(os.str());
  return ReadAggFile(is);
}

//------------------------------------------------------------------------
//                  GameAggRep: General data access
//------------------------------------------------------------------------

bool GameAggRep::IsConstSum(void) const
{
  AggPureStrategyProfileRep profile(const_cast<GameAggRep *>(this));

  Rational sum(0);
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    sum += profile.GetPayoff(pl);
  }

  for (StrategyIterator iter(StrategySupport(const_cast<GameAggRep *>(this)));
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
//                   GameAGGRep: Writing data files
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

void GameAggRep::WriteNfgFile(std::ostream &s) const{
	throw UndefinedException();
}
void GameAggRep::WriteAggFile(std::ostream &s) const{

	  //AGG identifier for Gambit ReadGame
	  s<<"#AGG"<<endl;

	  //num players

	  s<<aggPtr->getNumPlayers()<<endl;

	  //num action nodes

	  s<<aggPtr->getNumActionNodes()<<endl;

	  //num func nodes
	  s<<aggPtr->getNumFunctionNodes()<<endl;
	  //sizes of action sets
	  for(int i=0;i<aggPtr->getNumPlayers();++i){
		  s<<aggPtr->getNumActions(i)<< " ";
	  }
	  s<<endl;
	  //action sets
	  for(int i=0;i<aggPtr->getNumPlayers();++i){
		  std::copy(aggPtr->getActionSet(i).begin(), aggPtr->getActionSet(i).end(), std::ostream_iterator<int>(s," "));
		  s<<endl;
	  }
	  //action graph
	  aggPtr->printActionGraph(s);
	  s<<endl;

	  //types of function nodes
	  aggPtr->printTypes(s);
	  s<<endl;

	  //payoffs
	  for (int i=0;i<aggPtr->getNumActionNodes();++i){
	    s<<"1"<<endl; //type of payoff output
	    aggPtr->printPayoffs(s,i);
	    s<<endl;
	  }
}

GameAggRep* GameAggRep::ReadAggFile(istream& in){
	agg* aggPtr=agg::makeAGG(in);
	if(!aggPtr){
		throw InvalidFileException();
	}
	return new GameAggRep(aggPtr);
}

}  // end namespace Gambit
