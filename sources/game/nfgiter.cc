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

NfgIter::NfgIter(const NfgContIter &it)
  : support(it.support), m_nfg(it.m_nfg), current_strat(it.current_strat),
    profile(it.profile)
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

long NfgIter::GetIndex(void) const
{
  return profile.GetIndex();
}

gbtNfgOutcome NfgIter::GetOutcome(void) const
{
  return m_nfg.GetOutcome(profile);
}

void NfgIter::SetOutcome(gbtNfgOutcome outcome)
{
  m_nfg.SetOutcome(profile, outcome);
}


//-------------------------------------
// NfgContIter: Constructor, Destructor
//-------------------------------------

NfgContIter::NfgContIter(const gbtNfgSupport &s)
  : support(s), 
    current_strat(s.GetGame().NumPlayers()),
    m_nfg(s.GetGame()), profile(m_nfg), thawed(m_nfg.NumPlayers())
{
  for (int i = 1; i <= thawed.Length(); i++)
    thawed[i] = i;

  First();
}

NfgContIter::~NfgContIter()
{ }

//------------------------------
// NfgContIter: Member Functions
//------------------------------

void NfgContIter::First(void)
{
  for (int i = 1; i <= thawed.Length(); i++){
    profile.Set(thawed[i], support.GetStrategy(thawed[i], 1));
    current_strat[thawed[i]] = 1;
  }	
}

void NfgContIter::Set(int pl, int num)
{
  if (!frozen.Contains(pl))   return;

  profile.Set(pl, support.GetStrategy(pl, num));
  current_strat[pl] = num;
}

void NfgContIter::Set(gbtNfgStrategy s)
{
  if (!frozen.Contains(s.GetPlayer().GetId()))   return;

  profile.Set(s.GetPlayer().GetId(), s);
  current_strat[s.GetPlayer().GetId()] = s.GetId();
}

void NfgContIter::Freeze(const gBlock<int> &freeze)
{
  frozen = freeze;
  thawed = gBlock<int>(m_nfg.NumPlayers() - freeze.Length());
  for (int i = 1, j = 1; i <= m_nfg.NumPlayers(); i++)
    if (!frozen.Contains(i))   thawed[j++] = i;
  First();
}

void NfgContIter::Freeze(int pl)
{
  if (frozen.Contains(pl))   return;
  frozen.Append(pl);
  thawed.Remove(thawed.Find(pl));
  First();
}

void NfgContIter::Thaw(int pl)
{
  if (thawed.Contains(pl))   return;
  frozen.Remove(frozen.Find(pl));
  int i = 1;
  while (thawed[i] < pl)   i++;
  thawed.Insert(pl, i);
  First();
}

int NfgContIter::NextContingency(void)
{
  int j = thawed.Length();
  if (j == 0) return 0;    	

  while (1)   {
    int pl = thawed[j];
    if (current_strat[pl] < support.NumStrats(pl)) {
      gbtNfgStrategy s = support.GetStrategy(pl, ++(current_strat[pl]));
      profile.Set(pl, s);
      return 1;
    }
    profile.Set(pl, support.GetStrategy(pl, 1));
    current_strat[pl] = 1;
    j--;
    if (j == 0) {
      return 0;
    }
  }
}

long NfgContIter::GetIndex(void) const
{
  return profile.GetIndex();
}

const StrategyProfile &NfgContIter::Profile(void) const
{
  return profile;
}

gArray<int> NfgContIter::Get(void) const
{
  gArray<int> current(m_nfg.NumPlayers());
  for (int i = 1; i <= current.Length(); i++) {
    current[i] = profile[i].GetId();
  }
  return current;
}

void NfgContIter::Get(gArray<int> &t) const
{
  for (int i = 1; i <= m_nfg.NumPlayers(); i++) {
    t[i] = profile[i].GetId();
  }
}

gbtNfgOutcome NfgContIter::GetOutcome(void) const
{
  return m_nfg.GetOutcome(profile);
}

void NfgContIter::SetOutcome(gbtNfgOutcome outcome)
{
  m_nfg.SetOutcome(profile, outcome);
}

void NfgContIter::Dump(gOutput &f) const
{
  f << "{ ";
  for (int i = 1; i <= m_nfg.NumPlayers(); i++) {
    f << profile[i].GetId() << ' ';
  }
  f << '}';
}


