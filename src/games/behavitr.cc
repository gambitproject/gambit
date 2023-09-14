//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org
//
// FILE: src/libgambit/behavitr.cc
// Implementation of extensive form action iterators
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

BehaviorProfileIterator::BehaviorProfileIterator(const BehaviorSupportProfile &p_support)
  : m_atEnd(false), m_support(p_support),
    m_currentBehav(p_support.GetGame()->NumInfosets()),
    m_profile(p_support.GetGame()), 
    m_frozenPlayer(0), m_frozenInfoset(0),
    m_numActiveInfosets(m_support.GetGame()->NumPlayers())
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = m_support.GetGame()->GetPlayer(pl);
    m_numActiveInfosets[pl] = 0;
    Array<bool> activeForPl(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      activeForPl[iset] = m_support.MayReach(player->GetInfoset(iset));
      m_numActiveInfosets[pl]++;
    }
    m_isActive.push_back(activeForPl);
  }
  First();
}

BehaviorProfileIterator::BehaviorProfileIterator(const BehaviorSupportProfile &p_support,
			     const GameAction &p_action)
  : m_atEnd(false), m_support(p_support),
    m_currentBehav(p_support.GetGame()->NumInfosets()),
    m_profile(p_support.GetGame()), 
    m_frozenPlayer(p_action->GetInfoset()->GetPlayer()->GetNumber()), 
    m_frozenInfoset(p_action->GetInfoset()->GetNumber()),
    m_numActiveInfosets(m_support.GetGame()->NumPlayers())
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = m_support.GetGame()->GetPlayer(pl);
    m_numActiveInfosets[pl] = 0;
    Array<bool> activeForPl(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      activeForPl[iset] = p_support.MayReach(player->GetInfoset(iset));
      m_numActiveInfosets[pl]++;
    }
    m_isActive.push_back(activeForPl);
  }

  m_currentBehav(p_action->GetInfoset()->GetPlayer()->GetNumber(), 
		 p_action->GetInfoset()->GetNumber()) = p_support.GetIndex(p_action);
  m_profile.SetAction(p_action);
  First();
}

void BehaviorProfileIterator::First()
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= m_support.GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      if (pl != m_frozenPlayer && iset != m_frozenInfoset) {
	m_currentBehav(pl, iset) = 1;
	if (m_isActive[pl][iset]) {
	  m_profile.SetAction(m_support.GetAction(pl, iset, 1));
	}
      }
    }
  }
}

void BehaviorProfileIterator::operator++()
{
  int pl = m_support.GetGame()->NumPlayers();
  while (pl > 0 && m_numActiveInfosets[pl] == 0)
    --pl;
  if (pl == 0) {
    m_atEnd = true; 
    return;
  }

  int iset = m_support.GetGame()->GetPlayer(pl)->NumInfosets();
    
  while (true) {
    if (m_isActive[pl][iset] && 
	(pl != m_frozenPlayer || iset != m_frozenInfoset)) {
      if (m_currentBehav(pl, iset) < m_support.NumActions(pl, iset))  {
	m_profile.SetAction(m_support.GetAction(pl, iset, 
						++m_currentBehav(pl, iset)));
	return;
      }
      else {
	m_currentBehav(pl, iset) = 1;
	m_profile.SetAction(m_support.GetAction(pl, iset, 1));
      }
    }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && m_numActiveInfosets[pl] == 0);
      
      if (pl == 0) {
	m_atEnd = true;
	return;
      }
      iset = m_support.GetGame()->GetPlayer(pl)->NumInfosets();
    }
  }
}

} // end namespace Gambit
