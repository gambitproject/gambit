//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of players for table-represented games
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

#include "game.h"
#include "table-game.h"

//======================================================================
//            Implementation of class gbtTableStrategyRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTableStrategyRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTableStrategyRep::gbtTableStrategyRep(gbtTableInfosetRep *p_infoset,
					 int p_id)
  : m_refCount(0), m_id(p_id), 
    m_infoset(p_infoset), m_deleted(false), m_index(0L)
{ }

gbtTableStrategyRep::~gbtTableStrategyRep()
{ }

//----------------------------------------------------------------------
//     class gbtTableStrategyRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTableStrategyRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_infoset->m_player->m_nfg->m_refCount++;
}

bool gbtTableStrategyRep::Dereference(void)
{
  if (!m_deleted && --m_infoset->m_player->m_nfg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the strategy to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_infoset->m_player->m_nfg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//  class gbtTableStrategyRep: General information about the strategy
//----------------------------------------------------------------------

int gbtTableStrategyRep::GetId(void) const
{ return m_id; }

void gbtTableStrategyRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTableStrategyRep::GetLabel(void) const
{ return m_label; }

//----------------------------------------------------------------------
//  class gbtTableStrategyRep: Accessing information about the player
//----------------------------------------------------------------------

gbtGamePlayer gbtTableStrategyRep::GetPlayer(void) const
{ return m_infoset->m_player; }

//======================================================================
//            Implementation of class gbtTableInfosetRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTableInfosetRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTableInfosetRep::gbtTableInfosetRep(gbtTablePlayerRep *p_player,
					 int p_id, int p_br)
  : m_id(p_id), m_player(p_player), m_deleted(false),
    m_actions(p_br)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbtTableStrategyRep(this, act);
  }
}    

gbtTableInfosetRep::~gbtTableInfosetRep()
{
  // delete the actions
}

//----------------------------------------------------------------------
//     class gbtTableInfosetRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTableInfosetRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_player->m_nfg->m_refCount++;
}

bool gbtTableInfosetRep::Dereference(void)
{
  if (!m_deleted && --m_player->m_nfg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the infoset to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_player->m_nfg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//   class gbtTableInfosetRep: Accessing information about the player
//----------------------------------------------------------------------

gbtGamePlayer gbtTableInfosetRep::GetPlayer(void) const
{ return m_player; }

//======================================================================
//             Implementation of class gbtTablePlayerRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTablePlayerRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTablePlayerRep::gbtTablePlayerRep(gbtTableGameRep *p_nfg,
				     int p_id, int p_strats)
  : m_refCount(0), m_id(p_id), m_nfg(p_nfg), m_deleted(false)
{
  m_infosets.Append(new gbtTableInfosetRep(this, 1, p_strats));
}

//----------------------------------------------------------------------
//     class gbtTablePlayerRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTablePlayerRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_nfg->m_refCount++;
}

bool gbtTablePlayerRep::Dereference(void)
{
  if (!m_deleted && --m_nfg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the player to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_nfg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//    class gbtTablePlayerRep: General information about the player
//----------------------------------------------------------------------

int gbtTablePlayerRep::GetId(void) const
{ return m_id; }

void gbtTablePlayerRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTablePlayerRep::GetLabel(void) const
{ return m_label; }

bool gbtTablePlayerRep::IsDeleted(void) const
{ return m_deleted; }

//----------------------------------------------------------------------
// class gbtTablePlayerRep: Accessing the information sets of the player
//----------------------------------------------------------------------

int gbtTablePlayerRep::NumInfosets(void) const
{ return 1; }

//----------------------------------------------------------------------
//   class gbtTablePlayerRep: Accessing the sequences of the player
//----------------------------------------------------------------------

int gbtTablePlayerRep::NumSequences(void) const
{ return m_infosets[1]->m_actions.Length() + 1; }

//----------------------------------------------------------------------
//   class gbtTablePlayerRep: Accessing the strategies of the player
//----------------------------------------------------------------------

int gbtTablePlayerRep::NumStrategies(void) const
{ return m_infosets[1]->m_actions.Length(); }

gbtGameStrategy gbtTablePlayerRep::GetStrategy(int st) const
{ return m_infosets[1]->m_actions[st]; }
