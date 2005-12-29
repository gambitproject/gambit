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

//===========================================================================
//                    class gbtNfgContingencyIterator
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

gbtNfgContingencyIterator::gbtNfgContingencyIterator(const gbtNfgSupport &s)
  : support(s),
    m_currentStrat(s.GetGame()->NumPlayers()),
    profile(s.GetGame()), m_frozen1(0), m_frozen2(0)
{
  First();
}

gbtNfgContingencyIterator::gbtNfgContingencyIterator(const gbtNfgSupport &s,
						     int pl, int st)
  : support(s), 
    m_currentStrat(s.GetGame()->NumPlayers()),
    profile(s.GetGame()), m_frozen1(pl), m_frozen2(0)
{
  m_currentStrat[pl] = st;
  profile.SetStrategy(support.GetStrategy(pl, st));
  First();
}

gbtNfgContingencyIterator::gbtNfgContingencyIterator(const gbtNfgSupport &s,
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

void gbtNfgContingencyIterator::First(void)
{
  for (int pl = 1; pl <= support.GetGame()->NumPlayers(); pl++) {
    if (pl == m_frozen1 || pl == m_frozen2) continue;
    profile.SetStrategy(support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
  }	
}

bool gbtNfgContingencyIterator::NextContingency(void)
{
  int pl = support.GetGame()->NumPlayers();

  while (1)   {
    if (pl == m_frozen1 || pl == m_frozen2) {
      pl--;
      if (pl == 0)  return false;
      continue;
    }

    if (m_currentStrat[pl] < support.NumStrats(pl)) {
      profile.SetStrategy(support.GetStrategy(pl, ++(m_currentStrat[pl])));
      return true;
    }
    profile.SetStrategy(support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
    pl--;
    if (pl == 0)  return false;
  }
}

//---------------------------------------------------------------------------
//                              Data access
//---------------------------------------------------------------------------

Gambit::GameOutcome gbtNfgContingencyIterator::GetOutcome(void) const
{
  return profile.GetOutcome();
}

void gbtNfgContingencyIterator::SetOutcome(Gambit::GameOutcome outcome)
{
  profile.SetOutcome(outcome);
}

gbtRational gbtNfgContingencyIterator::GetPayoff(int pl) const
{
  if (profile.GetOutcome()) {
    return profile.GetOutcome()->GetPayoff(pl);
  }
  else {
    return gbtRational(0);
  }
}
