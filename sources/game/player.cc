//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of API for players
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

// Declaration of game API
#include "efg.h"
#include "nfg.h"

// Declaration of internal game classes
#include "efgint.h"
#include "nfgint.h"

//
// This file contains the implementation of the API for players.
// Hopefully, the concept of "player" will be shared by extensive and
// normal forms very soon.
//

gbtEfgPlayer gbtEfgStrategyBase::GetPlayer(void) const
{ return m_player; }

//----------------------------------------------------------------------
//           class gbtEfgPlayerBase: Member functions
//----------------------------------------------------------------------

gbtEfgPlayerBase::gbtEfgPlayerBase(gbt_efg_game_rep *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg)
{ }


gbtEfgPlayerBase::~gbtEfgPlayerBase()
{
  // Temporarily we will leak these information sets while API is in
  // transition.
  /*
  while (m_infosets.Length()) {
    delete m_infosets.Remove(1);
  }
  */
}

gbtEfgInfoset gbtEfgPlayerBase::NewInfoset(int p_actions)
{
  if (p_actions <= 0) {
    throw gbtEfgbtException();
  }
  return m_efg->NewInfoset(this, this->m_infosets.Length() + 1, p_actions);
}


gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgPlayer &)
{ 
  return p_stream;
}


//----------------------------------------------------------------------
//           gbtEfgPlayerIterator: Member function definitions
//----------------------------------------------------------------------

gbtEfgPlayerIterator::gbtEfgPlayerIterator(const gbtEfgGame &p_efg)
  : m_index(1), m_efg(p_efg)
{ }

gbtEfgPlayer gbtEfgPlayerIterator::operator*(void) const
{ return m_efg.GetPlayer(m_index); }

gbtEfgPlayerIterator &gbtEfgPlayerIterator::operator++(int)
{ m_index++; return *this; }

bool gbtEfgPlayerIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtEfgPlayerIterator::End(void) const
{ return m_index > m_efg.NumPlayers(); }

//----------------------------------------------------------------------
//           gbtEfgInfosetIterator: Member function definitions
//----------------------------------------------------------------------

gbtEfgInfosetIterator::gbtEfgInfosetIterator(const gbtEfgPlayer &p_player)
  : m_index(1), m_player(p_player)
{ }

gbtEfgInfoset gbtEfgInfosetIterator::operator*(void) const
{ return m_player->GetInfoset(m_index); }

gbtEfgInfosetIterator &gbtEfgInfosetIterator::operator++(int)
{ m_index++; return *this; }

bool gbtEfgInfosetIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtEfgInfosetIterator::End(void) const
{ return m_index > m_player->NumInfosets(); }


//----------------------------------------------------------------------
//                 gbtNfgPlayerBase: Declaration
//----------------------------------------------------------------------

gbtNfgPlayerBase::gbtNfgPlayerBase(gbt_nfg_game_rep *p_nfg,
				   int p_id, int p_strats)
  : m_id(p_id), m_nfg(p_nfg), m_deleted(false), m_refCount(0)
{
  m_infosets.Append(new gbt_nfg_infoset_rep(this, 1, p_strats));
}

