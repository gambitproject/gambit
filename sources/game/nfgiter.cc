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
#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"

//--------------------------------------------------------------------------
// NfgIter:  Constructors, Destructors, Operators
//--------------------------------------------------------------------------

NfgIter::NfgIter(gbtNfgGame p_nfg)
  : support(p_nfg),
    m_nfg(p_nfg), current_strat(p_nfg.NumPlayers()), profile(p_nfg)
{
  First();
}

NfgIter::NfgIter(const gbtNfgSupport &s) 
  : support(s), m_nfg(s.GetGame()),
    current_strat(m_nfg.NumPlayers()), profile(m_nfg)
{
  First();
}

NfgIter::NfgIter(const NfgIter &it)
  : support(it.support), m_nfg(it.m_nfg), current_strat(it.current_strat), 
    profile(it.profile)
{ }

NfgIter::NfgIter(const gbtNfgContIterator &p_iterator)
  : support(p_iterator.m_support), m_nfg(p_iterator.m_nfg), 
    current_strat(p_iterator.m_current), profile(p_iterator.m_profile)
{ }

NfgIter::~NfgIter()
{ }

NfgIter &NfgIter::operator=(const NfgIter &it)
{
  if (this != &it)  {
    m_nfg = it.m_nfg;
    profile = it.profile;
    current_strat = it.current_strat;
    support = it.support;
  }
  return *this;
}

//-----------------------------
// NfgIter: Member Functions
//-----------------------------

void NfgIter::First(void)
{
  for (int i = 1; i <= m_nfg.NumPlayers(); i++)  {
    gbtNfgStrategy s = support.GetStrategy(i, 1);
    profile.Set(i, s);
    current_strat[i] = 1;
  }
}

int NfgIter::Next(int p)
{
  if (current_strat[p] < support.NumStrats(p))  {
    gbtNfgStrategy s = support.GetStrategy(p, ++(current_strat[p]));
    profile.Set(p, s);
    return 1;
  }
  else {
    gbtNfgStrategy s = support.GetStrategy(p, 1);
    profile.Set(p, s);
    current_strat[p] = 1;
    return 0;
  }
}

int NfgIter::Set(int p, int s)
{
  if (p <= 0 || p > m_nfg.NumPlayers() ||
      s <= 0 || s > support.NumStrats(p))
    return 0;
  
  profile.Set(p, support.GetStrategy(p, s));
  return 1;
}

void NfgIter::Get(gArray<int> &t) const
{
  for (int i = 1; i <= m_nfg.NumPlayers(); i++) {
    t[i] = profile[i].GetId();
  }
}

void NfgIter::Set(const gArray<int> &t)
{
  for (int i = 1; i <= m_nfg.NumPlayers(); i++){
    profile.Set(i, support.GetStrategy(i, t[i]));
    current_strat[i] = t[i];
  } 
}

gbtNfgOutcome NfgIter::GetOutcome(void) const
{
  return profile.GetOutcome();
}

void NfgIter::SetOutcome(gbtNfgOutcome outcome)
{
  profile.SetOutcome(outcome);
}


//-------------------------------------
// gbtNfgContIterator: Constructor, Destructor
//-------------------------------------

gbtNfgContIterator::gbtNfgContIterator(const gbtNfgSupport &p_support)
  : m_support(p_support), 
    m_current(m_support.GetGame().NumPlayers()),
    m_nfg(m_support.GetGame()), m_profile(m_nfg), m_thawed(m_nfg.NumPlayers())
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
    m_profile.Set(m_thawed[i], m_support.GetStrategy(m_thawed[i], 1));
    m_current[m_thawed[i]] = 1;
  }	
}

void gbtNfgContIterator::Freeze(gbtNfgStrategy p_strategy)
{
  int player = p_strategy.GetPlayer().GetId();
  if (!m_frozen.Contains(player)) {
    m_frozen.Append(player);
    m_thawed.Remove(m_thawed.Find(player));
  }

  m_profile.Set(player, p_strategy);
  m_current[player] = p_strategy.GetId();
  First();
}

void gbtNfgContIterator::Thaw(gbtNfgPlayer p_player)
{
  int pl = p_player.GetId();
  if (m_thawed.Contains(pl)) {
    return;
  }
  m_frozen.Remove(m_frozen.Find(pl));
  int i = 1;
  while (m_thawed[i] < pl)   i++;
  m_thawed.Insert(pl, i);
  First();
}

int gbtNfgContIterator::Next(gbtNfgPlayer p_player)
{
  int p = p_player.GetId();
  if (!m_thawed.Contains(p)) {
    return 0;
  }

  if (m_current[p] < m_support.NumStrats(p))  {
    gbtNfgStrategy s = m_support.GetStrategy(p, ++(m_current[p]));
    m_profile.Set(p, s);
    First();
    return 1;
  }
  else {
    gbtNfgStrategy s = m_support.GetStrategy(p, 1);
    m_profile.Set(p, s);
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
    if (m_current[pl] < m_support.NumStrats(pl)) {
      gbtNfgStrategy s = m_support.GetStrategy(pl, ++(m_current[pl]));
      m_profile.Set(pl, s);
      return 1;
    }
    m_profile.Set(pl, m_support.GetStrategy(pl, 1));
    m_current[pl] = 1;
    j--;
    if (j == 0) {
      return 0;
    }
  }
}

