//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form contingency class
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

#include "nfgcont.h"
#include "nfgsupport.h"
#include "gamebase.h"

//-------------------------------------------------------------------------
//                    gbtNfgContingency member functions
//-------------------------------------------------------------------------

gbtNfgContingency::gbtNfgContingency(const gbtGame &p_nfg)
  : m_nfg(p_nfg), m_index(0L), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
    m_index += m_profile[pl]->GetIndex();
  }
}

gbtNfgContingency::gbtNfgContingency(const gbtNfgContingency &p)
  : m_nfg(p.m_nfg), m_index(p.m_index), m_profile(p.m_profile)
{ }

gbtNfgContingency::~gbtNfgContingency()
{ }

gbtNfgContingency &gbtNfgContingency::operator=(const gbtNfgContingency &p)
{
  if (this != &p) {
    m_nfg = p.m_nfg;
    m_index = p.m_index;
    m_profile = p.m_profile;
  }
  return *this;  
}

bool gbtNfgContingency::operator==(const gbtNfgContingency &p_cont) const
{
  if (m_nfg != p_cont.m_nfg) {
    return false;
  }
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    if (m_profile[pl] != p_cont.m_profile[pl]) {
      return false;
    }
  }
  return true;
}

void gbtNfgContingency::SetStrategy(gbtGameStrategy p_strategy)
{
  int pl = p_strategy->GetPlayer()->GetId();
  m_index += p_strategy->GetIndex() - m_profile[pl]->GetIndex();
  m_profile[pl] = p_strategy;
}

void gbtNfgContingency::SetOutcome(const gbtGameOutcome &p_outcome) const
{
  dynamic_cast<gbtGameBase *>(m_nfg.Get())->m_results[m_index + 1] = dynamic_cast<gbtGameOutcomeBase *>(p_outcome.Get());
  dynamic_cast<gbtGameBase *>(m_nfg.Get())->m_revision++;
}

gbtGameOutcome gbtNfgContingency::GetOutcome(void) const
{
  return dynamic_cast<gbtGameBase *>(m_nfg.Get())->m_results[m_index + 1];
}

gbtNumber gbtNfgContingency::GetPayoff(const gbtGamePlayer &p_player) const
{
  gbtGameBase *rep = dynamic_cast<gbtGameBase *>(m_nfg.Get());
  if (rep->m_results.Length() > 0) {
    return rep->m_results[m_index + 1]->m_payoffs[p_player->GetId()];
  }
  else {
    gbtArray<gbtArray<int> > behav(m_nfg->NumPlayers());
    for (int pl = 1; pl <= behav.Length(); pl++) {
      behav[pl] = m_profile[pl]->GetBehavior();
    }
    gbtVector<gbtNumber> payoff(m_nfg->NumPlayers());
    rep->Payoff(behav, payoff);
    return payoff[p_player->GetId()];
  }
}

