//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of iterator class for strategies in a support
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

#include "nfg.h"
#include "striter.h"

//========================================================================
//                     class gbtStrategyIterator
//========================================================================

//------------------------------------------------------------------------
//                  gbtStrategyIterator: Lifecycle
//------------------------------------------------------------------------

gbtStrategyIterator::gbtStrategyIterator(const gbtNfgSupport &p_support)
  : m_support(p_support), pl(1), st(0)
{
  // The idea behind this initialization is to guard against the
  // possibility that the support has no actions for player 1.
  // Changes to GoToNext() might screw this up.
  GoToNext();
}

//------------------------------------------------------------------------
//                  gbtStrategyIterator: Operators
//------------------------------------------------------------------------

bool gbtStrategyIterator::operator==(const gbtStrategyIterator &p_iter) const
{
  return (m_support == p_iter.m_support && pl == p_iter.pl && st == p_iter.st);
}

//------------------------------------------------------------------------
//                  gbtStrategyIterator: Iteration
//------------------------------------------------------------------------

bool gbtStrategyIterator::GoToNext(void)
{
  if (st != m_support.NumStrats(pl)) {
    st++; 
    return true;
  }
  else if (pl != m_support.GetGame().NumPlayers()) {
    pl++; 
    st = 1; 
    return true;
  }
  else  {
    return false;
  }
}

//------------------------------------------------------------------------
//             gbtStrategyIterator: Access to current state
//------------------------------------------------------------------------

gbtNfgStrategy gbtStrategyIterator::GetStrategy(void) const
{
  return m_support.GetStrategy(pl, st);
}

gbtNfgPlayer gbtStrategyIterator::GetPlayer(void) const
{
  return m_support.GetGame().GetPlayer(pl);
}

bool gbtStrategyIterator::IsLast(void) const
{
  return (pl == m_support.GetGame().NumPlayers() &&
	  st == m_support.NumStrats(pl)); 
}

bool gbtStrategyIterator::IsSubsequentTo(gbtNfgStrategy p_strategy) const
{
  if (pl > p_strategy.GetPlayer().GetId()) {
    return true; 
  }
  else if (pl < p_strategy.GetPlayer().GetId()) {
    return false;
  }
  else {
    return (st > p_strategy.GetId());
  }
}
