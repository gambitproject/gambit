//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of API for outcomes
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

//
// This file contains the implementation of the API for outcomes.
// Hopefully, the concept of "outcome" will be shared by extensive and
// normal forms very soon.
//

// Declaration of game API
#include "efg.h"

// Declaration of internal extensive form classes
#include "efgint.h"

//----------------------------------------------------------------------
//           struct gbt_efg_outcome_rep: Member functions
//----------------------------------------------------------------------

gbt_efg_outcome_rep::gbt_efg_outcome_rep(efgGame *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg), m_deleted(false), 
    m_payoffs(p_efg->NumPlayers()), m_doublePayoffs(p_efg->NumPlayers()),
    m_refCount(1)
{
  for (int i = 1; i <= m_payoffs.Length(); i++) {
    m_payoffs[i] = 0;
    m_doublePayoffs[i] = 0.0;
  }
}

//----------------------------------------------------------------------
//               class gbtEfgOutcome: Member functions
//----------------------------------------------------------------------

gbtEfgOutcome::gbtEfgOutcome(void)
  : rep(0)
{ }

gbtEfgOutcome::gbtEfgOutcome(gbt_efg_outcome_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgOutcome::gbtEfgOutcome(const gbtEfgOutcome &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgOutcome::~gbtEfgOutcome()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      delete rep;
    }
  }
}

gbtEfgOutcome &gbtEfgOutcome::operator=(const gbtEfgOutcome &p_outcome)
{
  if (this == &p_outcome) {
    return *this;
  }

  if (rep && --rep->m_refCount == 0) {
    delete rep;
  }

  if ((rep = p_outcome.rep) != 0) {
    rep->m_refCount++;
  }
  return *this;
}

bool gbtEfgOutcome::operator==(const gbtEfgOutcome &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtEfgOutcome::operator!=(const gbtEfgOutcome &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

bool gbtEfgOutcome::IsNull(void) const
{
  return (rep == 0);
}

efgGame *gbtEfgOutcome::GetGame(void) const
{
  return (rep) ? rep->m_efg : 0;
}

gText gbtEfgOutcome::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtEfgOutcome::SetLabel(const gText &p_label) 
{
  if (rep) {
    rep->m_label = p_label;
  }
}

gNumber gbtEfgOutcome::GetPayoff(const gbtEfgPlayer &p_player) const
{
  if (!rep || p_player.IsNull()) {
    return gNumber(0);
  }

  return rep->m_payoffs[p_player.GetId()];
}


gOutput &operator<<(gOutput &p_stream, const gbtEfgOutcome &)
{ 
  return p_stream;
}

