//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of strategy contingencies for explicit game trees
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

#include "tree-game.h"
#include "tree-contingency.h"
#include "table-game.h"

//---------------------------------------------------------------------------
//            Implementation of gbtNfgContingencyTree
//---------------------------------------------------------------------------

gbtNfgContingencyTree::gbtNfgContingencyTree(gbtTreeGameRep *p_efg)
  : m_efg(p_efg), m_profile(m_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)   {
    m_profile[pl] = m_efg->m_players[pl]->m_strategies[1];
  }
}

gbtNfgContingencyRep *gbtNfgContingencyTree::Copy(void) const
{
  gbtNfgContingencyTree *rep = new gbtNfgContingencyTree(m_efg);

  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    rep->m_profile[pl] = m_profile[pl];
  }
  return rep;
}

gbtGameStrategy 
gbtNfgContingencyTree::GetStrategy(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = 
    dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_efg) throw gbtGameMismatchException();
  return m_profile[player->m_id];
}

void gbtNfgContingencyTree::SetStrategy(const gbtGameStrategy &p_strategy)
{
  if (p_strategy.IsNull())  throw gbtGameNullException();
  gbtTreeStrategyRep *strategy = 
    dynamic_cast<gbtTreeStrategyRep *>(p_strategy.Get());
  if (!strategy || strategy->m_player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }
  
  m_profile[strategy->m_player->m_id] = strategy;
}

void gbtNfgContingencyTree::SetOutcome(const gbtGameOutcome &outcome)
{
  throw gbtGameUndefinedException();
}

gbtGameOutcome gbtNfgContingencyTree::GetOutcome(void) const
{
  throw gbtGameUndefinedException();
}

gbtRational 
gbtNfgContingencyTree::GetPayoff(const gbtGamePlayer &p_player) const
{
  gbtArray<gbtArray<int> *> behav(m_efg->NumPlayers());
  for (int pl = 1; pl <= behav.Length(); pl++) {
    behav[pl] = &m_profile[pl]->m_behav;
  }
  gbtVector<gbtRational> payoff(m_efg->NumPlayers());
  m_efg->Payoff(behav, payoff);
  return payoff[p_player->GetId()];
}

