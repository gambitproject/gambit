//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of players in explicit game trees
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
#include "tree-game.h"

//======================================================================
//            Implementation of class gbtTreeStrategyRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTreeStrategyRep: Constructor and destructor
//----------------------------------------------------------------------

gbtGamePlayer gbtTreeStrategyRep::GetPlayer(void) const
{ return m_player; }

gbtGameAction 
gbtTreeStrategyRep::GetBehavior(const gbtGameInfoset &p_infoset) const
{
  if (p_infoset.IsNull())  throw gbtGameNullException();
  gbtTreeInfosetRep *infoset = 
    dynamic_cast<gbtTreeInfosetRep *>(p_infoset.Get());
  if (!infoset || infoset->m_player != m_player) {
    throw gbtGameMismatchException();
  }
  return infoset->m_actions[m_behav[infoset->m_id]];
}

//======================================================================
//             Implementation of class gbtTreePlayerRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTreePlayerRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreePlayerRep::gbtTreePlayerRep(gbtTreeGameRep *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg), m_deleted(false)
{ }


gbtTreePlayerRep::~gbtTreePlayerRep()
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
//    class gbtTreePlayerRep: General information about the player
//----------------------------------------------------------------------

int gbtTreePlayerRep::GetId(void) const  
{ return m_id; }

bool gbtTreePlayerRep::IsChance(void) const
{ return (m_id == 0); }

void gbtTreePlayerRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTreePlayerRep::GetLabel(void) const
{ return m_label; }

bool gbtTreePlayerRep::IsDeleted(void) const
{ return m_deleted; }


//----------------------------------------------------------------------
// class gbtTreePlayerRep: Accessing the information sets of the player
//----------------------------------------------------------------------

int gbtTreePlayerRep::NumInfosets(void) const
{ return m_infosets.Length(); }

gbtGameInfoset gbtTreePlayerRep::GetInfoset(int p_index) const
{ return m_infosets[p_index]; }

gbtGameInfoset gbtTreePlayerRep::NewInfoset(int p_actions)
{
  if (p_actions <= 0)  throw gbtGameUndefinedException();

  gbtTreeInfosetRep *s = new gbtTreeInfosetRep(this,
					       m_infosets.Length() + 1,
					       p_actions);
  m_infosets.Append(s);
  return s;
}

//----------------------------------------------------------------------
//    class gbtTreePlayerRep: Accessing the sequences of the player
//----------------------------------------------------------------------

int gbtTreePlayerRep::NumSequences(void) const
{
  int count = 1;
  for (int iset = 1; iset <= m_infosets.Length(); 
       count += m_infosets[iset++]->NumActions());
  return count;
}

//----------------------------------------------------------------------
//    class gbtTreePlayerRep: Accessing the strategies of the player
//----------------------------------------------------------------------

int gbtTreePlayerRep::NumStrategies(void) const
{
  if (!m_efg->m_hasStrategies)  m_efg->BuildReducedNfg();
  return m_strategies.Length();
}

gbtGameStrategy gbtTreePlayerRep::GetStrategy(int p_index) const
{
  if (!m_efg->m_hasStrategies)  m_efg->BuildReducedNfg();
  return m_strategies[p_index];
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


