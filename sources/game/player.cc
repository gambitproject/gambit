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

//----------------------------------------------------------------------
//           struct gbt_efg_player_rep: Member functions
//----------------------------------------------------------------------

gbt_efg_player_rep::gbt_efg_player_rep(gbt_efg_game_rep *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg), m_deleted(false), 
    m_refCount(0)
{ }


gbt_efg_player_rep::~gbt_efg_player_rep()
{
  // Temporarily we will leak these information sets while API is in
  // transition.
  /*
  while (m_infosets.Length()) {
    delete m_infosets.Remove(1);
  }
  */
}

//----------------------------------------------------------------------
//               class gbtEfgPlayer: Member functions
//----------------------------------------------------------------------

gbtEfgPlayer::gbtEfgPlayer(void)
  : rep(0)
{ }

gbtEfgPlayer::gbtEfgPlayer(gbt_efg_player_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_efg->m_refCount++;
  }
}

gbtEfgPlayer::gbtEfgPlayer(const gbtEfgPlayer &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_efg->m_refCount++;
  }
}

gbtEfgPlayer::~gbtEfgPlayer()
{
  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      delete rep;
    }
    else if (--rep->m_efg->m_refCount == 0) {
      delete rep->m_efg;
    }
  }
}

gbtEfgPlayer &gbtEfgPlayer::operator=(const gbtEfgPlayer &p_player)
{
  if (this == &p_player) {
    return *this;
  }

  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      delete rep;
    }
    else if (--rep->m_efg->m_refCount == 0) {
      delete rep->m_efg;
    }
  }

  if ((rep = p_player.rep) != 0) {
    rep->m_refCount++;
    rep->m_efg->m_refCount++;
  }
  return *this;
}

bool gbtEfgPlayer::operator==(const gbtEfgPlayer &p_player) const
{
  return (rep == p_player.rep);
} 

bool gbtEfgPlayer::operator!=(const gbtEfgPlayer &p_player) const
{
  return (rep != p_player.rep);
} 

bool gbtEfgPlayer::IsNull(void) const
{
  return (rep == 0);
}

bool gbtEfgPlayer::IsDeleted(void) const
{
  return (rep && rep->m_deleted);
}

gbtEfgGame gbtEfgPlayer::GetGame(void) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep) ? rep->m_efg : 0;
  }
}

int gbtEfgPlayer::GetId(void) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep) ? rep->m_id : -1;
  }
}

gbtText gbtEfgPlayer::GetLabel(void) const
{
  if (rep) {
    if (rep->m_deleted) {
      throw gbtGameObjectDeleted();
    }
    else {
      return rep->m_label;
    }
  }
  else {
    return "";
  }
}

void gbtEfgPlayer::SetLabel(const gbtText &p_label)
{
  if (rep) {
    if (rep->m_deleted) {
      throw gbtGameObjectDeleted();
    }
    else {
      rep->m_label = p_label;
    }
  }
}

int gbtEfgPlayer::NumInfosets(void) const
{
  if (rep) {
    if (rep->m_deleted) {
      throw gbtGameObjectDeleted();
    }
    else {
      return rep->m_infosets.Length();
    }
  }
  else {
    return 0;
  }
}

gbtEfgInfoset gbtEfgPlayer::GetInfoset(int p_index) const
{
  if (rep) {
    if (rep->m_deleted) {
      throw gbtGameObjectDeleted();
    }
    else {
      return rep->m_infosets[p_index];
    }
  }
  else {
    return 0;
  }
}


bool gbtEfgPlayer::IsChance(void) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep && rep->m_id == 0);
  }
}

gbtEfgInfoset gbtEfgPlayer::NewInfoset(int p_actions)
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  if (IsNull())  {
    throw gbtEfgNullObject();
  }
  if (p_actions <= 0) {
    throw gbtEfgbtException();
  }
  return rep->m_efg->NewInfoset(rep, rep->m_infosets.Length() + 1, p_actions);
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
{ return m_player.GetInfoset(m_index); }

gbtEfgInfosetIterator &gbtEfgInfosetIterator::operator++(int)
{ m_index++; return *this; }

bool gbtEfgInfosetIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtEfgInfosetIterator::End(void) const
{ return m_index > m_player.NumInfosets(); }


//----------------------------------------------------------------------
//                 gbt_nfg_player_rep: Declaration
//----------------------------------------------------------------------

gbt_nfg_player_rep::gbt_nfg_player_rep(gbt_nfg_game_rep *p_nfg,
				       int p_id, int p_strats)
  : m_id(p_id), m_nfg(p_nfg), m_deleted(false), m_strategies(p_strats),
    m_refCount(0)
{
  for (int i = 1; i <= p_strats; i++) {
    m_strategies[i] = new gbt_nfg_strategy_rep(this);
  }
}

gbtNfgPlayer::gbtNfgPlayer(void)
  : rep(0)
{ }

gbtNfgPlayer::gbtNfgPlayer(gbt_nfg_player_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_nfg->m_refCount++;
  }
}

gbtNfgPlayer::gbtNfgPlayer(const gbtNfgPlayer &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_nfg->m_refCount++;
  }
}

gbtNfgPlayer::~gbtNfgPlayer()
{
  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      delete rep;
    }
    else if (--rep->m_nfg->m_refCount == 0) {
      delete rep->m_nfg;
    }
  }
}

gbtNfgPlayer &gbtNfgPlayer::operator=(const gbtNfgPlayer &p_player)
{
  if (this == &p_player) {
    return *this;
  }

  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      delete rep;
    }
    else if (--rep->m_nfg->m_refCount == 0) {
      delete rep->m_nfg;
    }
  }

  if ((rep = p_player.rep) != 0) {
    rep->m_refCount++;
    rep->m_nfg->m_refCount++;
  }
  return *this;
}

bool gbtNfgPlayer::operator==(const gbtNfgPlayer &p_player) const
{
  return (rep == p_player.rep);
} 

bool gbtNfgPlayer::operator!=(const gbtNfgPlayer &p_player) const
{
  return (rep != p_player.rep);
} 

int gbtNfgPlayer::GetId(void) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep) ? rep->m_id : 0;
  }
}

bool gbtNfgPlayer::IsNull(void) const
{
  return (rep == 0);
}

bool gbtNfgPlayer::IsDeleted(void) const
{
  return (rep && rep->m_deleted);
}

gbtNfgGame gbtNfgPlayer::GetGame(void) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep) ? rep->m_nfg : 0;
  }
}

gbtText gbtNfgPlayer::GetLabel(void) const
{
  if (rep) {
    if (rep->m_deleted) {
      throw gbtGameObjectDeleted();
    }
    else {
      return rep->m_label;
    }
  }
  else {
    return "";
  }
}

void gbtNfgPlayer::SetLabel(const gbtText &p_label)
{
  if (rep) {
    if (rep->m_deleted) {
      throw gbtGameObjectDeleted();
    }
    else {
      rep->m_label = p_label;
    }
  }
}

int gbtNfgPlayer::NumStrategies(void) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep) ? rep->m_strategies.Length() : 0;
  }
}

gbtNfgStrategy gbtNfgPlayer::GetStrategy(int st) const
{
  if (rep && rep->m_deleted) {
    throw gbtGameObjectDeleted();
  }
  else {
    return (rep) ? rep->m_strategies[st] : 0;
  }
}
