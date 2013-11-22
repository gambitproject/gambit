//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
  BagentPureStrategyProfileRep(const Game &p_game);
  virtual PureStrategyProfileRep *Copy(void) const;
  //virtual long GetIndex(void) const { return m_index; }
  virtual void SetStrategy(const GameStrategy &);
  virtual GameOutcome GetOutcome(void) const;
  virtual void SetOutcome(GameOutcome p_outcome);
  virtual Rational GetPayoff(int pl) const;
  virtual Rational GetStrategyValue(const GameStrategy &) const;
};

//------------------------------------------------------------------------
//               BaggPureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

BagentPureStrategyProfileRep::BagentPureStrategyProfileRep(const Game &p_game)
{

  m_nfg = p_game;
  m_profile = Array<GameStrategy>(p_game->NumPlayers());
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++)   {
    m_profile[pl] = p_game->GetPlayer(pl)->GetStrategy(1);
  }
}

PureStrategyProfileRep *BagentPureStrategyProfileRep::Copy(void) const
{
  return new BagentPureStrategyProfileRep(*this);
}
//------------------------------------------------------------------------
//       BagentPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void BagentPureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  //m_index += s->m_offset - m_profile[s->GetPlayer()->GetNumber()]->m_offset;
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

GameOutcome BagentPureStrategyProfileRep::GetOutcome(void) const
{
	throw UndefinedException();
}

void BagentPureStrategyProfileRep::SetOutcome(GameOutcome p_outcome)
{
	throw UndefinedException();
}

Rational BagentPureStrategyProfileRep::GetPayoff(int pl) const
{
	bagg* baggPtr = dynamic_cast<GameBagentRep &>(*m_nfg).baggPtr;
	int s[m_nfg->NumPlayers()];
	for(int p=1; p<=m_nfg->NumPlayers(); ++p){
		s[p-1] = m_profile[p]->GetNumber() -1;
	}
	int bp = dynamic_cast<GameBagentRep &>(*m_nfg).agent2baggPlayer[pl];
	int tp = pl - 1 - baggPtr->typeOffset[bp-1];
	return baggPtr->getPurePayoff(bp-1,tp,s);
}

Rational
BagentPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  bagg* baggPtr = dynamic_cast<GameBagentRep &>(*m_nfg).baggPtr;
  int s[m_nfg->NumPlayers()];
  for(int p=1; p<=m_nfg->NumPlayers(); ++p){
  		s[p-1] = m_profile[p]->GetNumber() -1;
  }
  s[player-1] = p_strategy->GetNumber() -1;

  int bp = dynamic_cast<GameBagentRep &>(*m_nfg).agent2baggPlayer[player];
  int tp = player - 1 - baggPtr->typeOffset[bp-1];
  return baggPtr->getPurePayoff(bp-1,tp,s);
}





//------------------------------------------------------------------------
//                   GameBagentRep: Lifecycle
//------------------------------------------------------------------------
Game GameBagentRep::Copy(void) const
{
  std::ostringstream os;
  WriteBaggFile(os);
  std::istringstream is(os.str());
  return ReadBaggFile(is);
}

//------------------------------------------------------------------------
//                   GameBagentRep: Factory functions
//------------------------------------------------------------------------

PureStrategyProfile GameBagentRep::NewPureStrategyProfile(void) const
{
    return PureStrategyProfile(new BagentPureStrategyProfileRep(const_cast<GameBagentRep *>(this)));
}

MixedStrategyProfile<double> GameBagentRep::NewMixedStrategyProfile(double) const
{
	//throw UndefinedException();
	return new BagentMixedStrategyProfileRep<double>(StrategySupport(const_cast<GameBagentRep *>(this)));
}

MixedStrategyProfile<Rational> GameBagentRep::NewMixedStrategyProfile(const Rational &) const
{
	//throw UndefinedException();
	return new BagentMixedStrategyProfileRep<Rational>(StrategySupport(const_cast<GameBagentRep *>(this)));
}
MixedStrategyProfile<double> GameBagentRep::NewMixedStrategyProfile(double, const StrategySupport& spt) const
{
	//throw UndefinedException();
	return new BagentMixedStrategyProfileRep<double>(spt);
}

MixedStrategyProfile<Rational> GameBagentRep::NewMixedStrategyProfile(const Rational &, const StrategySupport& spt) const
{
	//throw UndefinedException();
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

void GameBagentRep::WriteNfgFile(std::ostream &s) const{
	throw UndefinedException();
}
void GameBagentRep::WriteBaggFile(std::ostream &s) const{
    s<<(*baggPtr);
}

GameBagentRep* GameBagentRep::ReadBaggFile(istream& in){
	bagg* baggPtr=bagg::makeBAGG(in);
	if(!baggPtr){
		throw InvalidFileException();
	}
	return new GameBagentRep(baggPtr);
}






//------------------------------------------------------------------------
//                   GameBaggRep: Lifecycle
//------------------------------------------------------------------------
Game GameBaggRep::Copy(void) const
{
  std::ostringstream os;
  WriteBaggFile(os);
  std::istringstream is(os.str());
  return ReadBaggFile(is);
}
//------------------------------------------------------------------------
//                   GameBaggRep: Factory Functions
//------------------------------------------------------------------------
MixedStrategyProfile<double> GameBaggRep::NewMixedStrategyProfile(double) const
{
	throw UndefinedException();
}

MixedStrategyProfile<Rational> GameBaggRep::NewMixedStrategyProfile(const Rational &) const
{
	throw UndefinedException();
}
MixedStrategyProfile<double> GameBaggRep::NewMixedStrategyProfile(double, const StrategySupport& spt) const
{
	throw UndefinedException();
}

MixedStrategyProfile<Rational> GameBaggRep::NewMixedStrategyProfile(const Rational &, const StrategySupport& spt) const
{
	throw UndefinedException();
}
//------------------------------------------------------------------------
//                   GameBaggRep: Writing data files
//------------------------------------------------------------------------

void GameBaggRep::Write(std::ostream &p_stream,
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

void GameBaggRep::WriteNfgFile(std::ostream &s) const{
	throw UndefinedException();
}
void GameBaggRep::WriteBaggFile(std::ostream &s) const{
    s<<(*baggPtr);
}

GameBaggRep* GameBaggRep::ReadBaggFile(istream& in){
	bagg* baggPtr=bagg::makeBAGG(in);
	if(!baggPtr){
		throw InvalidFileException();
	}
	return new GameBaggRep(baggPtr);
}

}  // end namespace Gambit

