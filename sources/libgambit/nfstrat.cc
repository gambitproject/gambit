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

#include "libgambit.h"

//===========================================================================
//                        class gbtStrategyProfile
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

gbtStrategyProfile::gbtStrategyProfile(const Gambit::GameTable &p_nfg)
  : m_index(0L), m_nfg(p_nfg), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
    m_index += m_profile[pl]->m_index;
  }
}

//---------------------------------------------------------------------------
//                      Data access and manipulation
//---------------------------------------------------------------------------

void gbtStrategyProfile::SetStrategy(const Gambit::GameStrategy &s)
{
  if (!s) return;
  m_index += s->m_index - m_profile[s->GetPlayer()->GetNumber()]->m_index;
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Gambit::GameOutcome gbtStrategyProfile::GetOutcome(void) const
{ return m_nfg->results[m_index+1]; }

void gbtStrategyProfile::SetOutcome(Gambit::GameOutcome p_outcome)
{ m_nfg->results[m_index+1] = p_outcome; }

gbtRational gbtStrategyProfile::GetPayoff(int pl) const
{
  Gambit::GameOutcome outcome = GetOutcome();
  if (outcome) {
    return outcome->GetPayoff(pl);
  }
  else {
    return gbtRational(0);
  }
}

//===========================================================================
//                          class gbtNfgSupport
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

gbtNfgSupport::gbtNfgSupport(const Gambit::GameTable &p_nfg) 
  : m_nfg(p_nfg)
{ 
  for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
    m_support.Append(gbtArray<Gambit::GameStrategy>());
    for (int st = 1; st <= p_nfg->NumStrats(pl); st++) {
      m_support[pl].Append(p_nfg->GetPlayer(pl)->GetStrategy(st));
    }
  }
}

//---------------------------------------------------------------------------
//                          General information
//---------------------------------------------------------------------------

gbtArray<int> gbtNfgSupport::NumStrats(void) const
{
  gbtArray<int> a(m_support.Length());

  for (int pl = 1; pl <= a.Length(); pl++) {
    a[pl] = m_support[pl].Length();
  }
  return a;
}

int gbtNfgSupport::ProfileLength(void) const
{
  int total = 0;
  for (int pl = 1; pl <= m_nfg->NumPlayers();
       total += m_support[pl++].Length());
  return total;
}

bool gbtNfgSupport::IsSubsetOf(const gbtNfgSupport &p_support) const
{
  if (m_nfg != p_support.m_nfg)  return false;
  for (int pl = 1; pl <= m_support.Length(); pl++) {
    if (m_support[pl].Length() > p_support.m_support[pl].Length()) {
      return false;
    }
    else {
      for (int st = 1; st <= m_support[pl].Length(); st++) {
	if (!p_support.m_support[pl].Contains(m_support[pl][st])) {
	  return false;
	}
      }
    }
  }
  return true;
}

//---------------------------------------------------------------------------
//                        Modifying the support
//---------------------------------------------------------------------------

void gbtNfgSupport::AddStrategy(Gambit::GameStrategy s)
{ 
  gbtArray<Gambit::GameStrategy> &sup = m_support[s->GetPlayer()->GetNumber()];
  if (sup.Contains(s))  return;

  int index;
  for (index = 1; 
       index <= sup.Length() && sup[index]->GetNumber() < s->GetNumber(); 
       index++);
  sup.Insert(s, index);
}

bool gbtNfgSupport::RemoveStrategy(Gambit::GameStrategy s) 
{ 
  gbtArray<Gambit::GameStrategy> &sup = m_support[s->GetPlayer()->GetNumber()];
  if (!sup.Contains(s)) return false;
  if (sup.Contains(s) && sup.Length() == 1)  return false;
  sup.Remove(sup.Find(s));
  return true;
} 


