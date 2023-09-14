//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/stratitr.cc
// Implementation of normal form iterators
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

#include "gambit.h"

namespace Gambit {

//===========================================================================
//                        class StrategyProfileIterator
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support)
  : m_atEnd(false), m_support(p_support),
    m_currentStrat(m_support.GetGame()->NumPlayers()),
    m_profile(m_support.GetGame()->NewPureStrategyProfile()), 
    m_frozen1(0), m_frozen2(0)
{
  First();
}

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support,
						 int pl, int st)
  : m_atEnd(false), m_support(p_support), 
    m_currentStrat(m_support.GetGame()->NumPlayers()),
    m_profile(m_support.GetGame()->NewPureStrategyProfile()), 
    m_frozen1(pl), m_frozen2(0)
{
  m_currentStrat[pl] = st;
  m_profile->SetStrategy(m_support.GetStrategy(pl, st));
  First();
}

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support,
						 const GameStrategy &p_strategy)
  : m_atEnd(false), m_support(p_support),
    m_currentStrat(p_support.GetGame()->NumPlayers()),
    m_profile(p_support.GetGame()->NewPureStrategyProfile()), 
    m_frozen1(p_strategy->GetPlayer()->GetNumber()),
    m_frozen2(0)
{
  m_currentStrat[m_frozen1] = p_strategy->GetNumber();
  m_profile->SetStrategy(p_strategy);
  First();
}


StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support,
						 int pl1, int st1,
						 int pl2, int st2)
  : m_atEnd(false), m_support(p_support), 
    m_currentStrat(m_support.GetGame()->NumPlayers()),
    m_profile(m_support.GetGame()->NewPureStrategyProfile()), 
    m_frozen1(pl1), m_frozen2(pl2)
{
  m_currentStrat[pl1] = st1;
  m_profile->SetStrategy(m_support.GetStrategy(pl1, st1));
  m_currentStrat[pl2] = st2;
  m_profile->SetStrategy(m_support.GetStrategy(pl2, st2));
  First();
}

//---------------------------------------------------------------------------
//                                Iteration
//---------------------------------------------------------------------------

void StrategyProfileIterator::First()
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    if (pl == m_frozen1 || pl == m_frozen2) continue;
    m_profile->SetStrategy(m_support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
  }	
}

void StrategyProfileIterator::operator++()
{
  int pl = 1;

  while (true)   {
    if (pl == m_frozen1 || pl == m_frozen2) {
      pl++;
      if (pl > m_support.GetGame()->NumPlayers()) {
	m_atEnd = true;
	return;
      }
      continue;
    }

    if (m_currentStrat[pl] < m_support.NumStrategies(pl)) {
      m_profile->SetStrategy(m_support.GetStrategy(pl, ++(m_currentStrat[pl])));
      return;
    }
    m_profile->SetStrategy(m_support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
    pl++;
    if (pl > m_support.GetGame()->NumPlayers()) {
      m_atEnd = true;
      return;
    }
  }
}

} // end namespace Gambit
