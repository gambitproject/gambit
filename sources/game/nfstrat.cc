//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of strategy classes for normal forms
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

#include "base/base.h"
#include "nfstrat.h"
#include "nfg.h"

//--------------------------------------------------------
// StrategyProfile: Constructors, Destructors, Operators
//--------------------------------------------------------

StrategyProfile::StrategyProfile(const Nfg &N)
  : index(0L), profile(N.NumPlayers())
{
  for (int pl = 1; pl <= N.NumPlayers(); pl++)   {
    profile[pl] = N.GetPlayer(pl).GetStrategy(1);
    index += profile[pl]->GetIndex();
  }
}

StrategyProfile::StrategyProfile(const StrategyProfile &p)
: index(p.index), profile(p.profile)
{ }

StrategyProfile::~StrategyProfile()
{ }

StrategyProfile &StrategyProfile::operator=(const StrategyProfile &p)
{
  if (this != &p) {
    index = p.index;
    profile = p.profile;
  }
  return *this;  
}

//-----------------------------------------
// StrategyProfile: Members
//-----------------------------------------

bool StrategyProfile::IsValid(void) const
{
  int i;
  for (i = profile.Length(); i > 0 && profile[i] != 0; i--);
  return i;
}

long StrategyProfile::GetIndex(void) const 
{ 
  return index; 
}

Strategy *const StrategyProfile::operator[](int p) const 
{ 
  return profile[p];
}

Strategy *const StrategyProfile::Get(int p) const 
{ 
return profile[p];
}

void StrategyProfile::Set(int p, const Strategy *const s)
{
  index += (((s) ? s->GetIndex() : 0L) - ((profile[p]) ? profile[p]->GetIndex() : 0L));
  profile[p] = (Strategy *)s;
}

//==========================================================================
//                         class gbtNfgSupport
//==========================================================================

//--------------------------------------------------------------------------
//                   class gbtNfgSupport: Lifecycle
//--------------------------------------------------------------------------

gbtNfgSupport::gbtNfgSupport(const Nfg &p_nfg)
  : m_nfg(&p_nfg), m_strategies(p_nfg.NumStrats())
{ 
  // Initially, all strategies are contained in the support
  m_strategies = 1;
}

gbtNfgSupport &gbtNfgSupport::operator=(const gbtNfgSupport &p_support)
{
  if (this != &p_support && m_nfg == p_support.m_nfg) {
    m_name = p_support.m_name;
    m_strategies = p_support.m_strategies;
  }
  return *this;
}

//--------------------------------------------------------------------------
//                   class gbtNfgSupport: Operators
//--------------------------------------------------------------------------

bool gbtNfgSupport::operator==(const gbtNfgSupport &p_support) const
{
  return (m_nfg == p_support.m_nfg && m_strategies == p_support.m_strategies);
}
  
//--------------------------------------------------------------------------
//            class gbtNfgSupport: Data access -- strategies
//--------------------------------------------------------------------------

int gbtNfgSupport::NumStrats(int pl) const
{
  int total = 0;
  for (int st = 1; st <= m_strategies.Lengths()[pl]; st++) {
    total += m_strategies(pl, st);
  }
  return total;
}

gArray<int> gbtNfgSupport::NumStrats(void) const
{
  gArray<int> ret(m_nfg->NumPlayers());

  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = NumStrats(pl);
  }
  return ret;
}

int gbtNfgSupport::ProfileLength(void) const
{
  int total = 0;
  for (int i = 1; i <= m_strategies.Length(); i++) {
    total += m_strategies[i];
  }
  return total;
}

Strategy *gbtNfgSupport::GetStrategy(int pl, int st) const
{
  int index = 0;
  for (int i = 1; i <= m_nfg->NumStrats(pl); i++) {
    if (m_strategies(pl, i)) {
      if (++index == st) {
	return m_nfg->GetPlayer(pl).GetStrategy(i);
      }
    }
    
  }
  return 0;
}

int gbtNfgSupport::GetIndex(const Strategy *p_strategy) const
{
  int pl = p_strategy->GetPlayer().GetId();
  for (int st = 1; st <= NumStrats(pl); st++) {
    if (GetStrategy(pl, st) == p_strategy) {
      return st;
    }
  }
  return 0;
}

bool gbtNfgSupport::Contains(const Strategy *p_strategy) const
{
  return m_strategies(p_strategy->GetPlayer().GetId(), p_strategy->GetId());
}

//--------------------------------------------------------------------------
//                  class gbtNfgSupport: Manipulation
//--------------------------------------------------------------------------

void gbtNfgSupport::AddStrategy(Strategy *s)
{
  m_strategies(s->GetPlayer().GetId(), s->GetId()) = 1;
}

void gbtNfgSupport::RemoveStrategy(Strategy *s)
{
  m_strategies(s->GetPlayer().GetId(), s->GetId()) = 0;
}

//--------------------------------------------------------------------------
//           class gbtNfgSupport: Data access -- properties
//--------------------------------------------------------------------------

bool gbtNfgSupport::IsSubset(const gbtNfgSupport &s) const
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

bool gbtNfgSupport::IsValid(void) const
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    if (NumStrats(pl) == 0) {
      return false;
    }
  }
  return true;
}

void gbtNfgSupport::Output(gOutput &p_output) const
{
  p_output << '"' << m_name << "\" { ";
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    p_output << "{ ";
    for (int st = 1; st <= NumStrats(pl); st++) {
      p_output << "\"" << GetStrategy(pl, st)->GetLabel() << "\" ";
    }
    p_output << "} ";
  }
  p_output << "} ";
}

gOutput& operator<<(gOutput &p_stream, const gbtNfgSupport &p_support)
{
  p_support.Output(p_stream);
  return p_stream;
}

