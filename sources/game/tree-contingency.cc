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

//======================================================================
//          Implementation of class gbtTreeContingencyRep
//======================================================================

//----------------------------------------------------------------------
//      class gbtTreeContingencyRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeContingencyRep::gbtTreeContingencyRep(gbtTreeGameRep *p_efg)
  : m_refCount(0), m_efg(p_efg), m_profile(m_efg->NumPlayers())
{
  if (!m_efg->m_hasComputed)  m_efg->BuildComputedElements();

  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)   {
    m_profile[pl] = m_efg->m_players[pl]->m_strategies[1];
  }
}

gbtGameContingencyRep *gbtTreeContingencyRep::Copy(void) const
{
  gbtTreeContingencyRep *rep = new gbtTreeContingencyRep(m_efg);

  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    rep->m_profile[pl] = m_profile[pl];
  }
  return rep;
}

//----------------------------------------------------------------------
//   class gbtTreeContingencyRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeContingencyRep::Reference(void)
{
  m_refCount++;
  m_efg->m_refCount++;
}

bool gbtTreeContingencyRep::Dereference(void)
{
  if (--m_efg->m_refCount == 0) {
    // Note that if this condition is true, this profile must be the
    // last reference to the game, so m_refCount will also be one
    // (and this function will return true)
    delete m_efg;
  }
  return (--m_refCount == 0);
}

//----------------------------------------------------------------------
//        class gbtTreeContingencyRep: Accessing the state
//----------------------------------------------------------------------

gbtGameStrategy 
gbtTreeContingencyRep::GetStrategy(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = 
    dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_efg) throw gbtGameMismatchException();
  return m_profile[player->m_id];
}

void gbtTreeContingencyRep::SetStrategy(const gbtGameStrategy &p_strategy)
{
  if (p_strategy.IsNull())  throw gbtGameNullException();
  gbtTreeStrategyRep *strategy = 
    dynamic_cast<gbtTreeStrategyRep *>(p_strategy.Get());
  if (!strategy || strategy->m_player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }
  
  m_profile[strategy->m_player->m_id] = strategy;
}

void gbtTreeContingencyRep::SetOutcome(const gbtGameOutcome &outcome)
{
  throw gbtGameUndefinedException();
}

gbtGameOutcome gbtTreeContingencyRep::GetOutcome(void) const
{
  throw gbtGameUndefinedException();
}

gbtRational 
gbtTreeContingencyRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }

  return m_efg->GetPayoff(player, m_profile);
}

//======================================================================
//       Implementation of class gbtTreeContingencyIteratorRep
//======================================================================

//----------------------------------------------------------------------
//    class gbtTreeContingencyIteratorRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeContingencyIteratorRep::gbtTreeContingencyIteratorRep(gbtTreeGameRep *p_efg)
  : m_refCount(0), m_efg(p_efg), m_profile(m_efg->NumPlayers()),
    m_frozen(0)
{
  First();
}

gbtTreeContingencyIteratorRep::gbtTreeContingencyIteratorRep(gbtTreeGameRep *p_efg, gbtTreeStrategyRep *p_strategy)
  : m_refCount(0), m_efg(p_efg), m_profile(m_efg->NumPlayers()),
    m_frozen(p_strategy->m_player->m_id)
{
  m_profile[m_frozen] = p_strategy;
  First();
}

gbtTreeContingencyIteratorRep::~gbtTreeContingencyIteratorRep()
{ }

gbtGameContingencyIteratorRep *
gbtTreeContingencyIteratorRep::Copy(void) const
{
  gbtTreeContingencyIteratorRep *ret = 
    new gbtTreeContingencyIteratorRep(m_efg);
  ret->m_profile = m_profile;
  ret->m_frozen = m_frozen;
  return ret;
}

//----------------------------------------------------------------------
// class gbtTreeContingencyIteratorRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeContingencyIteratorRep::Reference(void)
{
  m_refCount++;
  m_efg->m_refCount++;
}

bool gbtTreeContingencyIteratorRep::Dereference(void)
{
  if (--m_efg->m_refCount == 0) {
    // Note that if this condition is true, this profile must be the
    // last reference to the game, so m_refCount will also be one
    // (and this function will return true)
    delete m_efg;
  }
  return (--m_refCount == 0);
}

//----------------------------------------------------------------------
//          class gbtTreeContingencyIteratorRep: Iteration
//----------------------------------------------------------------------

void gbtTreeContingencyIteratorRep::First(void)
{
  if (!m_efg->m_hasComputed)  m_efg->BuildComputedElements();

  for (int pl = 1; pl <= m_efg->m_players.Length(); pl++) {
    if (pl != m_frozen) {
      m_profile[pl] = m_efg->m_players[pl]->m_strategies[1];
    }
  }	
}

bool gbtTreeContingencyIteratorRep::NextContingency(void)
{
  int pl = m_efg->m_players.Length();

  while (1)   {
    if (pl == m_frozen) {
      if (--pl == 0) return false; else continue;
    }

    int st = m_profile[pl]->m_id;
    if (st < m_efg->m_players[pl]->m_strategies.Length()) {
      m_profile[pl] = m_efg->m_players[pl]->m_strategies[st + 1];
      return true;
    }
    m_profile[pl] = m_efg->m_players[pl]->m_strategies[1];
    if (--pl == 0) {
      return false;
    }
  }
}

//----------------------------------------------------------------------
//     class gbtTreeContingencyIteratorRep: Accessing the state
//----------------------------------------------------------------------

gbtGameStrategy 
gbtTreeContingencyIteratorRep::GetStrategy(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = 
    dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_efg) throw gbtGameMismatchException();
  return m_profile[player->m_id];
}

gbtGameOutcome gbtTreeContingencyIteratorRep::GetOutcome(void) const
{
  throw gbtGameUndefinedException();
}

gbtRational 
gbtTreeContingencyIteratorRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }

  return m_efg->GetPayoff(player, m_profile);
}

