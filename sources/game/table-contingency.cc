//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of strategy contingencies for table games
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

#include "table-game.h"
#include "table-contingency.h"

//---------------------------------------------------------------------------
//            Implementation of gbtNfgContingencyTable
//---------------------------------------------------------------------------

gbtNfgContingencyTable::gbtNfgContingencyTable(gbtTableGameRep *p_nfg)
  : m_nfg(p_nfg), m_index(0L), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->m_players[pl]->m_infosets[1]->m_actions[1];
    m_index += m_profile[pl]->m_index;
  }
}

gbtNfgContingencyRep *gbtNfgContingencyTable::Copy(void) const
{
  gbtNfgContingencyTable *rep = new gbtNfgContingencyTable(m_nfg);

  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    rep->m_profile[pl] = m_profile[pl];
  }
  rep->m_index = m_index;
  return rep;
}

gbtGameStrategy
gbtNfgContingencyTable::GetStrategy(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = 
    dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player || player->m_nfg != m_nfg) throw gbtGameMismatchException();
  return m_profile[player->m_id];
}


void gbtNfgContingencyTable::SetStrategy(const gbtGameStrategy &p_strategy)
{
  if (p_strategy.IsNull())  throw gbtGameNullException();
  gbtTableStrategyRep *strategy =
    dynamic_cast<gbtTableStrategyRep *>(p_strategy.Get());
  if (!strategy || strategy->m_infoset->m_player->m_nfg != m_nfg) {
    throw gbtGameMismatchException();
  }

  m_index += strategy->m_index - m_profile[strategy->m_infoset->m_player->m_id]->m_index;
  m_profile[strategy->m_infoset->m_player->m_id] = strategy;
}

void gbtNfgContingencyTable::SetOutcome(const gbtGameOutcome &p_outcome)
{
  if (p_outcome.IsNull()) {
    m_nfg->m_results[m_index + 1] = 0;
  }
  else {
    gbtTableOutcomeRep *outcome = 
      dynamic_cast<gbtTableOutcomeRep *>(p_outcome.Get());
    if (!outcome || outcome->m_nfg != m_nfg) throw gbtGameMismatchException();
    m_nfg->m_results[m_index + 1] = outcome;
  }
}

gbtGameOutcome gbtNfgContingencyTable::GetOutcome(void) const
{
  return m_nfg->m_results[m_index + 1];
}

gbtRational 
gbtNfgContingencyTable::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = 
    dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player || player->m_nfg != m_nfg)  throw gbtGameMismatchException();

  if (m_nfg->m_results[m_index + 1]) {
    return m_nfg->m_results[m_index + 1]->m_payoffs[p_player->GetId()];
  }
  else {
    return gbtRational(0);
  }
}

