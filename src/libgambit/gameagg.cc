//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gameagg.cc
// Implementation of strategic game representation
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
protected:
  long m_index;

public:
  AggPureStrategyProfileRep(const Game &p_game);
  virtual PureStrategyProfile Copy(void) const;
  virtual long GetIndex(void) const { return m_index; }
  virtual void SetStrategy(const GameStrategy &);
  virtual GameOutcome GetOutcome(void) const;
  virtual void SetOutcome(GameOutcome p_outcome);
  virtual Rational GetPayoff(int pl) const;
  virtual Rational GetStrategyValue(const GameStrategy &) const;
};

//------------------------------------------------------------------------
//               TablePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

AggPureStrategyProfileRep::AggPureStrategyProfileRep(const Game &p_game)
{
  m_index = 1L;
  m_nfg = p_game;
  m_profile = Array<GameStrategy>(p_game->NumPlayers());
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++)   {
    m_profile[pl] = p_game->GetPlayer(pl)->GetStrategy(1);
    //m_index += m_profile[pl]->m_offset;
  }
}

PureStrategyProfile AggPureStrategyProfileRep::Copy(void) const
{
  return PureStrategyProfile(new AggPureStrategyProfileRep(*this));
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
	agg* aggPtr = dynamic_cast<GameAGGRep &>(*m_nfg).aggPtr;
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
  agg* aggPtr = dynamic_cast<GameAGGRep &>(*m_nfg).aggPtr;
  int s[aggPtr->getNumPlayers()];
  for(int p=1; p<=aggPtr->getNumPlayers(); ++p){
  		s[p-1] = m_profile[p]->GetNumber() -1;
  }
  s[player-1] = p_strategy->GetNumber() -1;
  return aggPtr->getPurePayoff(player-1,s);
}



template <class T> class AggMixedStrategyProfileRep
  : public MixedStrategyProfileRep<T> {

public:
  AggMixedStrategyProfileRep(const StrategySupport &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  { }
  virtual ~AggMixedStrategyProfileRep() { }

  virtual MixedStrategyProfileRep<T> *Copy(void) const{
	  return new AggMixedStrategyProfileRep(*this);
  }
  virtual T GetPayoff(int pl) const{
	  agg* aggPtr = dynamic_cast<GameAGGRep &>(* (m_support.m_nfg)).aggPtr;
	  std::vector<T> s (aggPtr->getNumActions());
	  for (int i=0;i<aggPtr->getNumPlayers();++i)
		  for (int j=0;j<aggPtr->getNumActions(i);++j){
			  GameStrategy strategy = m_support.m_nfg->GetPlayer(i+1)->GetStrategy(j+1);
			  int ind=m_support.m_profileIndex[strategy];
			  s[aggPtr->firstAction(i)+j]= (ind==-1)?0:m_probs[ind];
		  }
	  return aggPtr->getMixedPayoff(pl-1, s);
  }
  virtual T GetPayoffDeriv(int pl, const GameStrategy &) const
  { throw UndefinedException(); }
  virtual T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const
  { throw UndefinedException(); }
};



//------------------------------------------------------------------------
//                   GameAGGRep: Factory functions
//------------------------------------------------------------------------

PureStrategyProfile GameAGGRep::NewPureStrategyProfile(void) const
{
    return PureStrategyProfile(new AggPureStrategyProfileRep(const_cast<GameAGGRep *>(this)));
}

MixedStrategyProfile<double> GameAGGRep::NewMixedStrategyProfile(double) const
{
  return new AggMixedStrategyProfileRep<double>(StrategySupport(const_cast<GameAGGRep *>(this)));
}

MixedStrategyProfile<Rational> GameAGGRep::NewMixedStrategyProfile(const Rational &) const
{
  return new AggMixedStrategyProfileRep<Rational>(StrategySupport(const_cast<GameAGGRep *>(this)));
}

}  // end namespace Gambit
