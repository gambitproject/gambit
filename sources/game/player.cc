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
#include "game.h"
#include "nfgsupport.h"

// Declaration of internal game classes
#include "gamebase.h"


//
// This file contains the implementation of the API for players.
// Hopefully, the concept of "player" will be shared by extensive and
// normal forms very soon.
//

gbtGamePlayer gbtGameStrategyBase::GetPlayer(void) const
{ return m_player; }

gbtGameAction
gbtGameStrategyBase::GetAction(const gbtGameInfoset &p_infoset) const
{ return p_infoset->GetAction(m_actions[p_infoset->GetId()]); }

long gbtGameStrategyBase::GetIndex(void) const { return m_index; }

//----------------------------------------------------------------------
//           class gbtGamePlayerBase: Member functions
//----------------------------------------------------------------------

gbtGamePlayerBase::gbtGamePlayerBase(gbtGameBase *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg)
{ }


gbtGamePlayerBase::~gbtGamePlayerBase()
{
  // Temporarily we will leak these information sets while API is in
  // transition.
  /*
  while (m_infosets.Length()) {
    delete m_infosets.Remove(1);
  }
  */
}

gbtGameInfoset gbtGamePlayerBase::NewInfoset(int p_actions)
{
  if (p_actions <= 0) {
    throw gbtGameException();
  }
  return m_efg->NewInfoset(this, this->m_infosets.Length() + 1, p_actions);
}

gbtGameInfoset gbtGamePlayerBase::GetInfoset(int p_index) const
{ return m_infosets[p_index]; }

gbtOutput &operator<<(gbtOutput &p_stream, const gbtGamePlayer &)
{ 
  return p_stream;
}


//----------------------------------------------------------------------
//           gbtGamePlayerIterator: Member function definitions
//----------------------------------------------------------------------

gbtGamePlayerIterator::gbtGamePlayerIterator(const gbtGame &p_efg)
  : m_index(1), m_efg(p_efg)
{ }

gbtGamePlayer gbtGamePlayerIterator::operator*(void) const
{ return m_efg->GetPlayer(m_index); }

gbtGamePlayerIterator &gbtGamePlayerIterator::operator++(int)
{ m_index++; return *this; }

bool gbtGamePlayerIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtGamePlayerIterator::End(void) const
{ return m_index > m_efg->NumPlayers(); }

//----------------------------------------------------------------------
//           gbtGameInfosetIterator: Member function definitions
//----------------------------------------------------------------------

gbtGameInfosetIterator::gbtGameInfosetIterator(const gbtGamePlayer &p_player)
  : m_index(1), m_player(p_player)
{ }

gbtGameInfoset gbtGameInfosetIterator::operator*(void) const
{ return m_player->GetInfoset(m_index); }

gbtGameInfosetIterator &gbtGameInfosetIterator::operator++(int)
{ m_index++; return *this; }

bool gbtGameInfosetIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtGameInfosetIterator::End(void) const
{ return m_index > m_player->NumInfosets(); }
