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

#include "efg.h"
#include "actiter.h"

//========================================================================
//                     class gbtActionIterator
//========================================================================

//------------------------------------------------------------------------
//                  gbtActionIterator: Lifecycle
//------------------------------------------------------------------------

gbtActionIterator::gbtActionIterator(const EFSupport &p_support)
  : m_support(p_support), pl(1), iset(1), act(0)
{
  // The idea behind this initialization is to guard against the
  // possibility that the support has no actions in player 1's first
  // information set.  Changes to GoToNext() might screw this up.
  GoToNext();
}

//------------------------------------------------------------------------
//                  gbtActionIterator: Operators
//------------------------------------------------------------------------

bool gbtActionIterator::operator==(const gbtActionIterator &p_iter) const
{
  return (m_support == p_iter.m_support && pl == p_iter.pl &&
	  iset == p_iter.iset && act == p_iter.act);
}

//------------------------------------------------------------------------
//                  gbtActionIterator: Iteration
//------------------------------------------------------------------------

bool gbtActionIterator::GoToNext(void)
{
  if (act != m_support.NumActions(pl,iset)) {
    act++; 
    return true;
  }
  
  int temppl = pl;
  int tempiset = iset + 1;

  while (temppl <= m_support.GetGame().NumPlayers()) {
    while (tempiset <= m_support.GetGame().GetPlayer(temppl).NumInfosets()) {
      if (m_support.NumActions(temppl,tempiset) > 0) {
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
//              gbtActionIterator: Access to current state
//------------------------------------------------------------------------

Action *gbtActionIterator::GetAction(void) const
{
  return m_support.Actions(pl, iset)[act];
}

Infoset *gbtActionIterator::GetInfoset(void) const
{
  return m_support.GetGame().GetPlayer(pl).GetInfoset(iset);
}

gbtEfgPlayer gbtActionIterator::GetPlayer(void) const
{
  return m_support.GetGame().GetPlayer(pl);
}

bool gbtActionIterator::IsLast(void) const
{
  return (pl == m_support.GetGame().NumPlayers() &&
	  iset == m_support.GetGame().GetPlayer(pl).NumInfosets() &&
	  act == m_support.NumActions(pl, iset));
}

bool gbtActionIterator::IsSubsequentTo(const Action *p_action) const
{
  if (pl > p_action->BelongsTo()->GetPlayer().GetId()) {
    return true; 
  }
  else if (pl < p_action->BelongsTo()->GetPlayer().GetId()) {
    return false;
  }
  else {
    if (iset > p_action->BelongsTo()->GetNumber()) {
      return true; 
    }
    else if (iset < p_action->BelongsTo()->GetNumber()) {
      return false;
    }
    else {
      return (act > p_action->GetNumber());
    }
  }
}

