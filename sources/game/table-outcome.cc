//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of outcomes for table games
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
#include "table-game.h"

//======================================================================
//            Implementation of class gbtTableOutcomeRep
//======================================================================

//----------------------------------------------------------------------
//       class gbtTableOutcomeRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTableOutcomeRep::gbtTableOutcomeRep(gbtTableGameRep *p_nfg, int p_id)
  : m_refCount(0), m_id(p_id), m_nfg(p_nfg), m_deleted(false), 
    m_payoffs(p_nfg->m_players.Length())
{
  for (int i = 1; i <= m_payoffs.Length(); m_payoffs[i++] = 0);
}

//----------------------------------------------------------------------
//     class gbtTableOutcomeRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTableOutcomeRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_nfg->m_refCount++;
}

bool gbtTableOutcomeRep::Dereference(void)
{
  if (!m_deleted && --m_nfg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the outcome to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_nfg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//   class gbtTableOutcomeRep: General information about the outcome
//----------------------------------------------------------------------

int gbtTableOutcomeRep::GetId(void) const
{ return m_id; }

void gbtTableOutcomeRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTableOutcomeRep::GetLabel(void) const
{ return m_label; }

bool gbtTableOutcomeRep::IsDeleted(void) const
{ return m_deleted; }

//----------------------------------------------------------------------
//  class gbtTableOutcomeRep: Accessing payoff info about the outcome
//----------------------------------------------------------------------

gbtArray<gbtRational> gbtTableOutcomeRep::GetPayoff(void) const
{ return m_payoffs; }

gbtRational gbtTableOutcomeRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player)  throw gbtGameMismatchException();
  return m_payoffs[player->m_id];
}

void gbtTableOutcomeRep::SetPayoff(const gbtGamePlayer &p_player,
				   const gbtRational &p_value)
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player)  throw gbtGameMismatchException();
  m_payoffs[player->m_id] = p_value;
}

//----------------------------------------------------------------------
//     class gbtTableOutcomeRep: Editing the outcomes in a game
//----------------------------------------------------------------------

void gbtTableOutcomeRep::DeleteOutcome(void)
{
  m_nfg->DeleteOutcome(this);
  m_deleted = true;
}

