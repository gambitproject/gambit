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
    profile[pl] = N.Strategies(pl)[1];
    index += profile[pl]->Index();
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
  index += (((s) ? s->Index() : 0L) - ((profile[p]) ? profile[p]->Index() : 0L));
  profile[p] = (Strategy *)s;
}

//-----------------------------------------------
// gbtNfgSupport: Ctors, Dtor, Operators
//-----------------------------------------------

gbtNfgSupport::gbtNfgSupport(const Nfg &N) : 
  bnfg(&N), m_strategies(N.NumStrats())
{ 
  m_strategies = 1;
}

gbtNfgSupport::gbtNfgSupport(const gbtNfgSupport &s)
  : bnfg(s.bnfg), m_strategies(s.m_strategies), m_name(s.m_name)
{ }

gbtNfgSupport::~gbtNfgSupport()
{ }

gbtNfgSupport &gbtNfgSupport::operator=(const gbtNfgSupport &s)
{
  if (this != &s && bnfg == s.bnfg) {
    m_name = s.m_name;
    m_strategies = s.m_strategies;
  }
  return *this;
}

bool gbtNfgSupport::operator==(const gbtNfgSupport &s) const
{
  return (bnfg == s.bnfg && m_strategies == s.m_strategies);
}
  
bool gbtNfgSupport::operator!=(const gbtNfgSupport &s) const
{
  return !(*this == s);
}

//------------------------
// gbtNfgSupport: Members
//------------------------

gArray<Strategy *> gbtNfgSupport::Strategies(int pl) const
{
  gBlock<Strategy *> ret;
  for (int st = 1; st <= bnfg->NumStrats(pl); st++) {
    if (m_strategies(pl, st)) {
      ret += bnfg->GetPlayer(pl).GetStrategy(st);
    }
  }
  return ret;
}

int gbtNfgSupport::GetNumber(const Strategy *s) const
{
  int pl = s->GetPlayer().GetId();
  gArray<Strategy *> strats = Strategies(pl);
  for (int i = 1; i <= strats.Length(); i++) {
    if (strats[i] == s) {
      return i;
    }
  }
  return 0;
}

int gbtNfgSupport::NumStrats(int pl) const
{
  return Strategies(pl).Length();
}

gArray<int> gbtNfgSupport::NumStrats(void) const
{
  gArray<int> ret(bnfg->NumPlayers());

  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = NumStrats(pl);
  }
  return ret;
}

int gbtNfgSupport::TotalNumStrats(void) const
{
  int total = 0;
  for (int i = 1; i <= m_strategies.Length(); i++) {
    total += m_strategies[i];
  }
  return total;
}

int gbtNfgSupport::Find(Strategy *s) const
{
  return GetNumber(s);
}

bool gbtNfgSupport::StrategyIsActive(Strategy *s) const
{
  if (Find(s) > 0)
    return true;
  else
    return false;
}

void gbtNfgSupport::AddStrategy(Strategy *s)
{
  m_strategies(s->GetPlayer().GetId(), s->Number()) = 1;
}

bool gbtNfgSupport::RemoveStrategy(Strategy *s)
{
  m_strategies(s->GetPlayer().GetId(), s->Number()) = 0;
  return false;
}

// Returns true if all strategies in this belong to s
bool gbtNfgSupport::IsSubset(const gbtNfgSupport &s) const
{
  if (bnfg != s.bnfg)  return false;
  for (int pl = 1; pl <= bnfg->NumPlayers(); pl++) {
    if (NumStrats(pl) > s.NumStrats(pl)) {
      return false;
    }

    for (int st = 1; st <= bnfg->NumStrats(pl); st++) {
      if (m_strategies(pl, st) && !s.m_strategies(pl, st)) {
	return false;
      }
    }
  }
  return true;
}

bool gbtNfgSupport::IsValid(void) const
{
  for (int pl = 1; pl <= bnfg->NumPlayers(); pl++) {
    if (NumStrats(pl) == 0) {
      return false;
    }
  }
  return true;
}

void gbtNfgSupport::Dump(gOutput &p_output) const
{
  p_output << '"' << m_name << "\" { ";
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    p_output << "{ ";
    const gArray<Strategy *> &strategies = Strategies(pl);
    for (int st = 1; st <= strategies.Length(); st++) {
      p_output << "\"" << strategies[st]->Name() << "\" ";
    }
    p_output << "} ";
  }
  p_output << "} ";
}

gOutput& operator<<(gOutput& s, const gbtNfgSupport& n)
{
  n.Dump(s);
  return s;
}

