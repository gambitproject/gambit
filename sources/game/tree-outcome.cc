//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of outcomes for explicit game trees
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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
#include "tree-game.h"

//======================================================================
//            Implementation of class gbtTreeOutcomeRep
//======================================================================

//----------------------------------------------------------------------
//       class gbtTreeOutcomeRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeOutcomeRep::gbtTreeOutcomeRep(gbtTreeGameRep *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg), m_deleted(false), 
    m_payoffs(p_efg->m_players.Length())
{
  for (int i = 1; i <= m_payoffs.Length(); m_payoffs[i++] = 0);
}

//----------------------------------------------------------------------
//    class gbtTreeOutcomeRep: General information about the outcome
//----------------------------------------------------------------------

int gbtTreeOutcomeRep::GetId(void) const
{ return m_id; }

void gbtTreeOutcomeRep::SetLabel(const std::string &p_label) 
{ m_label = p_label; }

std::string gbtTreeOutcomeRep::GetLabel(void) const
{ return m_label; }

//----------------------------------------------------------------------
//   class gbtTreeOutcomeRep: Accessing payoff info about the outcome
//----------------------------------------------------------------------

gbtArray<gbtRational> gbtTreeOutcomeRep::GetPayoff(void) const
{ return m_payoffs; }

gbtRational gbtTreeOutcomeRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player)   throw gbtGameMismatchException();
  return m_payoffs[player->m_id];
}

void gbtTreeOutcomeRep::SetPayoff(const gbtGamePlayer &p_player,
				  const gbtRational &p_value)
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player)   throw gbtGameMismatchException();
  m_payoffs[player->m_id] = p_value;
}

//----------------------------------------------------------------------
//      class gbtTreeOutcomeRep: Editing the outcomes in a game
//----------------------------------------------------------------------

void gbtTreeOutcomeRep::DeleteOutcome(void)
{
  m_efg->m_root->DeleteOutcome(this);
  m_efg->m_outcomes.Remove(m_efg->m_outcomes.Find(this));
  m_deleted = true;
  for (int outc = 1; outc <= m_efg->m_outcomes.Length(); outc++) {
    m_efg->m_outcomes[outc]->m_id = outc;
  }
}
