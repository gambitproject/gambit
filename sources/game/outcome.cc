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
#include "nfg.h"

// Declaration of internal game classes
#include "efgint.h"
#include "nfgint.h"

//----------------------------------------------------------------------
//            class gbtEfgOutcomeBase: Member functions
//----------------------------------------------------------------------

gbtEfgOutcomeBase::gbtEfgOutcomeBase(gbt_efg_game_rep *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg),
    m_payoffs(p_efg->players.Length()),
    m_doublePayoffs(p_efg->players.Length())
{
  for (int i = 1; i <= m_payoffs.Length(); i++) {
    m_payoffs[i] = 0;
    m_doublePayoffs[i] = 0.0;
  }
}

void gbtEfgOutcomeBase::DeleteOutcome(void)
{
  m_efg->DeleteOutcome(this);
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgOutcomeBase &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//                 gbtNfgOutcomeBase: Member functions
//----------------------------------------------------------------------

gbtNfgOutcomeBase::gbtNfgOutcomeBase(gbt_nfg_game_rep *p_nfg, int p_id)
  : m_id(p_id), m_nfg(p_nfg),
    m_payoffs(p_nfg->m_players.Length()),
    m_doublePayoffs(p_nfg->m_players.Length())
{
  for (int i = 1; i <= m_payoffs.Length(); i++) {
    m_payoffs[i] = 0;
    m_doublePayoffs[i] = 0.0;
  }
}

void gbtNfgOutcomeBase::DeleteOutcome(void)
{
  m_nfg->DeleteOutcome(this);
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtNfgOutcomeBase &)
{ 
  return p_stream;
}
