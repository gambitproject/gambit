//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of iterator class for actions in a support
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

#include "game.h"
#include "actiter.h"

//========================================================================
//                     class gbtAllActionIterator
//========================================================================

//------------------------------------------------------------------------
//                  gbtAllActionIterator: Lifecycle
//------------------------------------------------------------------------

gbtAllActionIterator::gbtAllActionIterator(const gbtEfgSupport &p_support)
  : m_support(p_support), pl(1), iset(1), act(0)
{
  // The idea behind this initialization is to guard against the
  // possibility that the support has no actions in player 1's first
  // information set.  Changes to GoToNext() might screw this up.
  GoToNext();
}

//------------------------------------------------------------------------
//                  gbtAllActionIterator: Operators
//------------------------------------------------------------------------

bool gbtAllActionIterator::operator==(const gbtAllActionIterator &p_iter) const
{
  return (m_support == p_iter.m_support && pl == p_iter.pl &&
	  iset == p_iter.iset && act == p_iter.act);
}

//------------------------------------------------------------------------
//                   gbtAllActionIterator: Iteration
//------------------------------------------------------------------------

bool gbtAllActionIterator::GoToNext(void)
{
  while (iset > m_support->GetPlayer(pl)->NumInfosets()) {
    pl++;
    iset = 1;
    if (pl > m_support->NumPlayers()) {
      return false;
    }
  }

  if (act != m_support->GetPlayer(pl)->GetInfoset(iset)->NumActions()) {
    act++; 
    return true;
  }
  
  int temppl = pl;
  int tempiset = iset + 1;

  while (temppl <= m_support->NumPlayers()) {
    while (tempiset <= m_support->GetPlayer(temppl)->NumInfosets()) {
      if (m_support->GetPlayer(temppl)->GetInfoset(tempiset)->NumActions() > 0) {
	pl = temppl;
	iset = tempiset;
	act = 1;
	return true;
      }
      else {
	tempiset++;
      }
    }
    tempiset = 1;
    temppl++;
  }
  return false;
}

//------------------------------------------------------------------------
//              gbtAllActionIterator: Access to current state
//------------------------------------------------------------------------

gbtGameAction gbtAllActionIterator::GetAction(void) const
{
  return m_support->GetAction(pl, iset, act);
}

gbtGameInfoset gbtAllActionIterator::GetInfoset(void) const
{
  return m_support->GetPlayer(pl)->GetInfoset(iset);
}

gbtGamePlayer gbtAllActionIterator::GetPlayer(void) const
{
  return m_support->GetPlayer(pl);
}

bool gbtAllActionIterator::IsLast(void) const
{
  return (pl == m_support->NumPlayers() &&
	  iset == m_support->GetPlayer(pl)->NumInfosets() &&
	  act == m_support->GetPlayer(pl)->GetInfoset(iset)->NumActions());
}

bool gbtAllActionIterator::IsSubsequentTo(const gbtGameAction &p_action) const
{
  if (pl > p_action->GetInfoset()->GetPlayer()->GetId()) {
    return true; 
  }
  else if (pl < p_action->GetInfoset()->GetPlayer()->GetId()) {
    return false;
  }
  else {
    if (iset > p_action->GetInfoset()->GetId()) {
      return true; 
    }
    else if (iset < p_action->GetInfoset()->GetId()) {
      return false;
    }
    else {
      return (act > p_action->GetId());
    }
  }
}

//========================================================================
//                     class gbtAllActionIterator
//========================================================================

gbtActionIterator::gbtActionIterator(const gbtEfgSupport &p_support,
				     gbtGameInfoset p_infoset)
  : m_support(p_support), pl(p_infoset->GetPlayer()->GetId()),
    iset(p_infoset->GetId()), act(1)
{ }

gbtActionIterator::gbtActionIterator(const gbtEfgSupport &p_support,
				     int p_player, int p_infoset)
  : m_support(p_support), pl(p_player), iset(p_infoset), act(1)
{ }

gbtGameAction gbtActionIterator::operator*(void) const
{ return m_support->GetAction(pl, iset, act); }

gbtActionIterator &gbtActionIterator::operator++(int)
{ act++; return *this; }

bool gbtActionIterator::Begin(void)
{ act = 1; return true; }

bool gbtActionIterator::End(void) const
{ return act > m_support->GetPlayer(pl)->GetInfoset(iset)->NumActions(); }
