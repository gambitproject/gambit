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

//======================================================================
//          Implementation of class gbtTableContingencyRep
//======================================================================

//----------------------------------------------------------------------
//      class gbtTableContingencyRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTableContingencyRep::gbtTableContingencyRep(gbtTableGameRep *p_nfg)
  : m_refCount(0), m_nfg(p_nfg), m_index(0L), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->m_players[pl]->m_infosets[1]->m_actions[1];
    m_index += m_profile[pl]->m_index;
  }
}

gbtGameContingencyRep *gbtTableContingencyRep::Copy(void) const
{
  gbtTableContingencyRep *rep = new gbtTableContingencyRep(m_nfg);

  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    rep->m_profile[pl] = m_profile[pl];
  }
  rep->m_index = m_index;
  return rep;
}

//----------------------------------------------------------------------
//   class gbtTableContingencyRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTableContingencyRep::Reference(void)
{
  m_refCount++;
  m_nfg->m_refCount++;
}

bool gbtTableContingencyRep::Dereference(void)
{
  if (--m_nfg->m_refCount == 0) {
    // Note that if this condition is true, this profile must be the
    // last reference to the game, so m_refCount will also be one
    // (and this function will return true)
    delete m_nfg;
  }
  return (--m_refCount == 0);
}

//----------------------------------------------------------------------
//        class gbtTableContingencyRep: Accessing the state
//----------------------------------------------------------------------

gbtGameStrategy
gbtTableContingencyRep::GetStrategy(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = 
    dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player || player->m_nfg != m_nfg) throw gbtGameMismatchException();
  return m_profile[player->m_id];
}


void gbtTableContingencyRep::SetStrategy(const gbtGameStrategy &p_strategy)
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

void gbtTableContingencyRep::SetOutcome(const gbtGameOutcome &p_outcome)
{
  if (p_outcome.IsNull()) {
    m_nfg->m_results[m_index + 1] = 0;
  }
  else {
    gbtTableOutcomeRep *outcome = 
      dynamic_cast<gbtTableOutcomeRep *>(p_outcome.Get());
    if (!outcome || outcome->m_nfg != m_nfg) throw gbtGameMismatchException();
    (*m_nfg->m_results)[m_index + 1] = outcome;
  }
}

gbtGameOutcome gbtTableContingencyRep::GetOutcome(void) const
{
  return (*m_nfg->m_results)[m_index + 1];
}

gbtRational 
gbtTableContingencyRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = 
    dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player || player->m_nfg != m_nfg)  throw gbtGameMismatchException();

  if ((*m_nfg->m_results)[m_index + 1]) {
    return (*m_nfg->m_results)[m_index + 1]->m_payoffs[p_player->GetId()];
  }
  else {
    return gbtRational(0);
  }
}

//======================================================================
//       Implementation of class gbtTableContingencyIteratorRep
//======================================================================

//----------------------------------------------------------------------
//    class gbtTableContingencyIteratorRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTableContingencyIteratorRep::gbtTableContingencyIteratorRep(gbtTableGameRep *p_nfg)
  : m_refCount(0), m_frozen(0), m_nfg(p_nfg), 
    m_index(0L), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_profile.Length(); m_profile[pl++] = 0);
  First();
}

gbtTableContingencyIteratorRep::gbtTableContingencyIteratorRep(gbtTableGameRep *p_nfg, gbtTableStrategyRep *p_strategy)
  : m_refCount(0), m_frozen(p_strategy->m_infoset->m_player->m_id),
    m_nfg(p_nfg), m_index(p_strategy->m_index), m_profile(m_nfg->NumPlayers())
{
  for (int pl = 1; pl <= m_profile.Length(); m_profile[pl++] = 0);
  m_profile[m_frozen] = p_strategy;
  First();
}

gbtTableContingencyIteratorRep::~gbtTableContingencyIteratorRep()
{ }

gbtGameContingencyIteratorRep *
gbtTableContingencyIteratorRep::Copy(void) const
{
  gbtTableContingencyIteratorRep *ret = 
    new gbtTableContingencyIteratorRep(m_nfg);
  ret->m_frozen = m_frozen;
  ret->m_profile = m_profile;
  ret->m_index = m_index;
  return ret;
}

//----------------------------------------------------------------------
// class gbtTableContingencyIteratorRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTableContingencyIteratorRep::Reference(void)
{
  m_refCount++;
  m_nfg->m_refCount++;
}

bool gbtTableContingencyIteratorRep::Dereference(void)
{
  if (--m_nfg->m_refCount == 0) {
    // Note that if this condition is true, this profile must be the
    // last reference to the game, so m_refCount will also be one
    // (and this function will return true)
    delete m_nfg;
  }
  return (--m_refCount == 0);
}

//----------------------------------------------------------------------
//         class gbtTableContingencyIteratorRep: Iteration
//----------------------------------------------------------------------

void gbtTableContingencyIteratorRep::First(void)
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    if (pl != m_frozen) {
      if (m_profile[pl])  m_index -= m_profile[pl]->m_index;
      m_profile[pl] = m_nfg->m_players[pl]->m_infosets[1]->m_actions[1];
      m_index += m_profile[pl]->m_index;
    }
  }	
}

bool gbtTableContingencyIteratorRep::NextContingency(void)
{
  int pl = m_nfg->NumPlayers();

  while (1)   {
    if (pl == m_frozen) {
      if (--pl == 0) return false; else continue;
    }

    int st = m_profile[pl]->m_id;
    if (st < m_nfg->m_players[pl]->m_infosets[1]->m_actions.Length()) {
      m_index -= m_profile[pl]->m_index;
      m_profile[pl] = m_nfg->m_players[pl]->m_infosets[1]->m_actions[st+1];
      m_index += m_profile[pl]->m_index;
      return true;
    }

    m_index -= m_profile[pl]->m_index;
    m_profile[pl] = m_nfg->m_players[pl]->m_infosets[1]->m_actions[1];
    m_index += m_profile[pl]->m_index;
    if (--pl == 0) {
      return false;
    }
  }
}

//----------------------------------------------------------------------
//     class gbtTableContingencyIteratorRep: Accessing the state
//----------------------------------------------------------------------

gbtGameStrategy 
gbtTableContingencyIteratorRep::GetStrategy(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = 
    dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player || player->m_nfg != m_nfg) throw gbtGameMismatchException();
  return m_profile[player->m_id];
}
  
gbtGameOutcome gbtTableContingencyIteratorRep::GetOutcome(void) const
{
  return (*m_nfg->m_results)[m_index + 1];
}

gbtRational 
gbtTableContingencyIteratorRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTablePlayerRep *player = 
    dynamic_cast<gbtTablePlayerRep *>(p_player.Get());
  if (!player || player->m_nfg != m_nfg)  throw gbtGameMismatchException();

  if ((*m_nfg->m_results)[m_index + 1]) {
    return (*m_nfg->m_results)[m_index + 1]->m_payoffs[p_player->GetId()];
  }
  else {
    return gbtRational(0);
  }
}
