//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of normal form iterators
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "libgambit.h"

namespace Gambit {

//===========================================================================
//                        class StrategyIterator
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

StrategyIterator::StrategyIterator(const StrategySupport &s)
  : support(s),
    m_currentStrat(s.GetGame()->NumPlayers()),
    profile(s.GetGame()), m_frozen1(0), m_frozen2(0)
{
  First();
}

StrategyIterator::StrategyIterator(const StrategySupport &s, int pl, int st)
  : support(s), 
    m_currentStrat(s.GetGame()->NumPlayers()),
    profile(s.GetGame()), m_frozen1(pl), m_frozen2(0)
{
  m_currentStrat[pl] = st;
  profile.SetStrategy(support.GetStrategy(pl, st));
  First();
}

StrategyIterator::StrategyIterator(const StrategySupport &p_support,
				   const GameStrategy &p_strategy)
  : support(p_support),
    m_currentStrat(p_support.GetGame()->NumPlayers()),
    profile(p_support.GetGame()), 
    m_frozen1(p_strategy->GetPlayer()->GetNumber()),
    m_frozen2(0)
{
  m_currentStrat[m_frozen1] = p_strategy->GetNumber();
  profile.SetStrategy(p_strategy);
  First();
}


StrategyIterator::StrategyIterator(const StrategySupport &s,
				   int pl1, int st1,
				   int pl2, int st2)
  : support(s), 
    m_currentStrat(s.GetGame()->NumPlayers()),
    profile(s.GetGame()), m_frozen1(pl1), m_frozen2(pl2)
{
  m_currentStrat[pl1] = st1;
  profile.SetStrategy(support.GetStrategy(pl1, st1));
  m_currentStrat[pl2] = st2;
  profile.SetStrategy(support.GetStrategy(pl2, st2));
  First();
}

//---------------------------------------------------------------------------
//                                Iteration
//---------------------------------------------------------------------------

void StrategyIterator::First(void)
{
  for (int pl = 1; pl <= support.GetGame()->NumPlayers(); pl++) {
    if (pl == m_frozen1 || pl == m_frozen2) continue;
    profile.SetStrategy(support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
  }	
}

bool StrategyIterator::NextContingency(void)
{
  int pl = 1;

  while (1)   {
    if (pl == m_frozen1 || pl == m_frozen2) {
      pl++;
      if (pl > support.GetGame()->NumPlayers())  return false;
      continue;
    }

    if (m_currentStrat[pl] < support.NumStrategies(pl)) {
      profile.SetStrategy(support.GetStrategy(pl, ++(m_currentStrat[pl])));
      return true;
    }
    profile.SetStrategy(support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
    pl++;
    if (pl > support.GetGame()->NumPlayers())  return false;
  }
}

//---------------------------------------------------------------------------
//                              Data access
//---------------------------------------------------------------------------

GameOutcome StrategyIterator::GetOutcome(void) const
{
  return profile.GetOutcome();
}

void StrategyIterator::SetOutcome(GameOutcome outcome)
{
  profile.SetOutcome(outcome);
}

Rational StrategyIterator::GetPayoff(int pl) const
{
  return profile.GetPayoff<Rational>(pl);
}

} // end namespace Gambit
