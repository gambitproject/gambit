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

#include "nfgiter.h"
#include "nfgciter.h"
#include "nfgsupport.h"

//--------------------------------------------------------------------------
// gbtNfgIterator:  Constructors, Destructors, Operators
//--------------------------------------------------------------------------

gbtNfgIterator::gbtNfgIterator(const gbtNfgGame &p_nfg)
  : m_nfg(p_nfg),
    m_current(m_nfg->NumPlayers()),
    m_profile(m_nfg->NewContingency())
{
  First();
}

gbtNfgIterator::gbtNfgIterator(const gbtNfgIterator &p_iterator)
  : m_nfg(p_iterator.m_nfg), 
    m_current(p_iterator.m_current),
    m_profile(p_iterator.m_profile)
{ }

gbtNfgIterator::gbtNfgIterator(const gbtNfgContIterator &p_iterator)
  : m_nfg(p_iterator.m_nfg),
    m_current(p_iterator.m_current),
    m_profile(p_iterator.m_profile)
{ }

gbtNfgIterator::~gbtNfgIterator()
{ }


//-----------------------------
// gbtNfgIterator: Member Functions
//-----------------------------

void gbtNfgIterator::First(void)
{
  for (int i = 1; i <= m_nfg->NumPlayers(); i++)  {
    gbtGameStrategy s = m_nfg->GetPlayer(i)->GetStrategy(1);
    m_profile->SetStrategy(s);
    m_current[i] = 1;
  }
}

int gbtNfgIterator::Next(int p)
{
  if (m_current[p] < m_nfg->GetPlayer(p)->NumStrategies())  {
    gbtGameStrategy s = m_nfg->GetPlayer(p)->GetStrategy(++(m_current[p]));
    m_profile->SetStrategy(s);
    return 1;
  }
  else {
    gbtGameStrategy s = m_nfg->GetPlayer(p)->GetStrategy(1);
    m_profile->SetStrategy(s);
    m_current[p] = 1;
    return 0;
  }
}

int gbtNfgIterator::Set(int p, int s)
{
  if (p <= 0 || p > m_nfg->NumPlayers() ||
      s <= 0 || s > m_nfg->GetPlayer(p)->NumStrategies())
    return 0;
  
  m_profile->SetStrategy(m_nfg->GetPlayer(p)->GetStrategy(s));
  return 1;
}

void gbtNfgIterator::Get(gbtArray<int> &t) const
{
  for (int i = 1; i <= m_nfg->NumPlayers(); i++) {
    t[i] = m_profile->GetStrategy(m_nfg->GetPlayer(i))->GetId();
  }
}

void gbtNfgIterator::Set(const gbtArray<int> &t)
{
  for (int i = 1; i <= m_nfg->NumPlayers(); i++){
    m_profile->SetStrategy(m_nfg->GetPlayer(i)->GetStrategy(t[i]));
    m_current[i] = t[i];
  } 
}

gbtGameOutcome gbtNfgIterator::GetOutcome(void) const
{
  return m_profile->GetOutcome();
}


//-------------------------------------
// gbtNfgContIterator: Constructor, Destructor
//-------------------------------------

gbtNfgContIterator::gbtNfgContIterator(const gbtNfgGame &p_nfg)
  : m_nfg(p_nfg),
    m_current(m_nfg->NumPlayers()),
    m_profile(m_nfg->NewContingency()),
    m_thawed(m_nfg->NumPlayers())
{
  for (int i = 1; i <= m_thawed.Length(); i++) {
    m_thawed[i] = i;
  }

  First();
}

gbtNfgContIterator::~gbtNfgContIterator()
{ }

//------------------------------
// gbtNfgContIterator: Member Functions
//------------------------------

void gbtNfgContIterator::First(void)
{
  for (int i = 1; i <= m_thawed.Length(); i++) {
    m_profile->SetStrategy(m_nfg->GetPlayer(m_thawed[i])->GetStrategy(1));
    m_current[m_thawed[i]] = 1;
  }	
}

void gbtNfgContIterator::Freeze(gbtGameStrategy p_strategy)
{
  int player = p_strategy->GetPlayer()->GetId();
  if (!m_frozen.Contains(player)) {
    m_frozen.Append(player);
    m_thawed.Remove(m_thawed.Find(player));
  }

  m_profile->SetStrategy(p_strategy);
  m_current[player] = p_strategy->GetId();
  First();
}

void gbtNfgContIterator::Thaw(gbtGamePlayer p_player)
{
  int pl = p_player->GetId();
  if (m_thawed.Contains(pl)) {
    return;
  }
  m_frozen.Remove(m_frozen.Find(pl));
  int i = 1;
  while (m_thawed[i] < pl)   i++;
  m_thawed.Insert(pl, i);
  First();
}

int gbtNfgContIterator::Next(gbtGamePlayer p_player)
{
  int p = p_player->GetId();
  if (!m_thawed.Contains(p)) {
    return 0;
  }

  if (m_current[p] < m_nfg->GetPlayer(p)->NumStrategies())  {
    gbtGameStrategy s = m_nfg->GetPlayer(p)->GetStrategy(++(m_current[p]));
    m_profile->SetStrategy(s);
    First();
    return 1;
  }
  else {
    gbtGameStrategy s = m_nfg->GetPlayer(p)->GetStrategy(1);
    m_profile->SetStrategy(s);
    m_current[p] = 1;
    First();
    return 0;
  }
}

int gbtNfgContIterator::NextContingency(void)
{
  int j = m_thawed.Length();
  if (j == 0) return 0;    	

  while (1)   {
    int pl = m_thawed[j];
    if (m_current[pl] < m_nfg->GetPlayer(pl)->NumStrategies()) {
      gbtGameStrategy s = m_nfg->GetPlayer(pl)->GetStrategy(++(m_current[pl]));
      m_profile->SetStrategy(s);
      return 1;
    }
    m_profile->SetStrategy(m_nfg->GetPlayer(pl)->GetStrategy(1));
    m_current[pl] = 1;
    j--;
    if (j == 0) {
      return 0;
    }
  }
}
