//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of support class for normal forms
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
#include "nfgsupport.h"
#include "gamebase.h"

//==========================================================================
//                         class gbtNfgSupportBase
//==========================================================================

//--------------------------------------------------------------------------
//                   class gbtNfgSupportBase: Lifecycle
//--------------------------------------------------------------------------

gbtNfgSupportBase::gbtNfgSupportBase(const gbtGame &p_nfg)
  : m_nfg(p_nfg), m_strategies(p_nfg->NumStrategies())
{ 
  // Initially, all strategies are contained in the support
  m_strategies = 1;
}

gbtNfgSupportRep *gbtNfgSupportBase::Copy(void) const
{
  return new gbtNfgSupportBase(*this);
}

//--------------------------------------------------------------------------
//                   class gbtNfgSupportBase: Operators
//--------------------------------------------------------------------------

bool gbtNfgSupportBase::operator==(const gbtNfgSupportRep &p_support) const
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int st = 1; st <= NumStrats(pl); st++) {
      bool term1 = m_strategies(pl, st);
      bool term2 = p_support.Contains(m_nfg->GetPlayer(pl)->GetStrategy(st));

      if (term1 != term2) {
	return false;
      }
    }
  }
  return true;
}
  
//--------------------------------------------------------------------------
//            class gbtNfgSupportBase: Data access -- strategies
//--------------------------------------------------------------------------

int gbtNfgSupportBase::NumStrats(int pl) const
{
  int total = 0;
  for (int st = 1; st <= m_strategies.Lengths()[pl]; st++) {
    total += m_strategies(pl, st);
  }
  return total;
}

gbtArray<int> gbtNfgSupportBase::NumStrategies(void) const
{
  gbtArray<int> ret(m_nfg->NumPlayers());

  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = NumStrats(pl);
  }
  return ret;
}

int gbtNfgSupportBase::MixedProfileLength(void) const
{
  int total = 0;
  for (int i = 1; i <= m_strategies.Length(); i++) {
    total += m_strategies[i];
  }
  return total;
}

gbtGameStrategy gbtNfgSupportBase::GetStrategy(int pl, int st) const
{
  int index = 0;
  for (int i = 1; i <= m_nfg->NumStrats(pl); i++) {
    if (m_strategies(pl, i)) {
      if (++index == st) {
	return m_nfg->GetPlayer(pl)->GetStrategy(i);
      }
    }
    
  }
  return 0;
}

int gbtNfgSupportBase::GetIndex(gbtGameStrategy p_strategy) const
{
  int pl = p_strategy->GetPlayer()->GetId();
  for (int st = 1; st <= NumStrats(pl); st++) {
    if (GetStrategy(pl, st) == p_strategy) {
      return st;
    }
  }
  return 0;
}

bool gbtNfgSupportBase::Contains(gbtGameStrategy p_strategy) const
{
  return m_strategies(p_strategy->GetPlayer()->GetId(), p_strategy->GetId());
}

//--------------------------------------------------------------------------
//                  class gbtNfgSupportBase: Manipulation
//--------------------------------------------------------------------------

void gbtNfgSupportBase::AddStrategy(gbtGameStrategy s)
{
  m_strategies(s->GetPlayer()->GetId(), s->GetId()) = 1;
}

void gbtNfgSupportBase::RemoveStrategy(gbtGameStrategy s)
{
  m_strategies(s->GetPlayer()->GetId(), s->GetId()) = 0;
}

//--------------------------------------------------------------------------
//           class gbtNfgSupportBase: Data access -- properties
//--------------------------------------------------------------------------

bool gbtNfgSupportBase::IsSubset(const gbtNfgSupportBase &s) const
{
  if (m_nfg != s.m_nfg)  return false;
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    if (NumStrats(pl) > s.NumStrats(pl)) {
      return false;
    }

    for (int st = 1; st <= m_nfg->NumStrats(pl); st++) {
      if (m_strategies(pl, st) && !s.m_strategies(pl, st)) {
	return false;
      }
    }
  }
  return true;
}

bool gbtNfgSupportBase::IsValid(void) const
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    if (NumStrats(pl) == 0) {
      return false;
    }
  }
  return true;
}

gbtOutput& operator<<(gbtOutput &p_stream, const gbtNfgSupport &p_support)
{
  return p_stream;
}
