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
#include "tree-contingency.h"

//======================================================================
//            Implementation of class gbtTreeStrategyRep
//======================================================================

//----------------------------------------------------------------------
//     class gbtTreeStrategyRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeStrategyRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_player->m_efg->m_refCount++;
}

bool gbtTreeStrategyRep::Dereference(void)
{
  if (!m_deleted && --m_player->m_efg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the strategy to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_player->m_efg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//  class gbtTreeStrategyRep: Accessing information about the player
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

//----------------------------------------------------------------------
//         class gbtTreeStrategyRep: Dominance properties
//----------------------------------------------------------------------

bool gbtTreeStrategyRep::Dominates(const gbtGameStrategy &p_strategy, 
				   bool p_strict) const
{
  if (p_strategy.IsNull()) throw gbtGameNullException();
  gbtTreeStrategyRep *strategy = 
    dynamic_cast<gbtTreeStrategyRep *>(p_strategy.Get());
  if (!strategy || strategy->m_player != m_player) {
    throw gbtGameMismatchException();
  }

  gbtTreeContingencyIteratorRep A(m_player->m_efg, 
				  const_cast<gbtTreeStrategyRep *>(this));
  gbtTreeContingencyIteratorRep B(m_player->m_efg, strategy);

  if (p_strict) {
    do  {
      if (A.GetPayoff(m_player) <= B.GetPayoff(m_player)) {
	return false;
      }
      A.NextContingency();
    } while (B.NextContingency());
	
    return true;
  }
  else {   // weak dominance
    bool equal = true;
  
    do   {
      gbtRational ap = A.GetPayoff(m_player);
      gbtRational bp = B.GetPayoff(m_player);

      if (ap < bp) { 
	return false;
      }
      else if (ap > bp) {
	equal = false;
      }
    } while (A.NextContingency() && B.NextContingency());
    
    return (!equal);
  }
}

bool gbtTreeStrategyRep::IsDominated(bool p_strict) const
{
  for (int st = 1; st <= m_player->m_strategies.Length(); st++) {
    if (st != m_id) {
      if (m_player->m_strategies[st]->Dominates(const_cast<gbtTreeStrategyRep *>(this), p_strict)) {
	return true;
      }
    }
  }
  return false;
}

//======================================================================
//            Implementation of class gbtTreeSequenceRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTreeSequenceRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeSequenceRep::gbtTreeSequenceRep(gbtTreePlayerRep *p_player,
				       gbtTreeActionRep *p_action,
				       gbtTreeSequenceRep *p_parent,
				       int p_id)
  : m_refCount(0), m_id(p_id), 
    m_player(p_player), m_action(p_action), m_parent(p_parent), 
    m_deleted(false)
{ }

//----------------------------------------------------------------------
//     class gbtTreeSequenceRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeSequenceRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_player->m_efg->m_refCount++;
}

bool gbtTreeSequenceRep::Dereference(void)
{
  if (!m_deleted && --m_player->m_efg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the strategy to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_player->m_efg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//   class gbtTreeSequenceRep: Accessing information about the player
//----------------------------------------------------------------------

gbtGamePlayer gbtTreeSequenceRep::GetPlayer(void) const
{ return m_player; }

gbtGameAction gbtTreeSequenceRep::GetAction(void) const
{ return m_action; }

gbtGameSequence gbtTreeSequenceRep::GetParent(void) const
{ return m_parent; }

bool gbtTreeSequenceRep::ContainsAction(const gbtGameAction &p_action) const
{
  if (p_action.IsNull()) throw gbtGameNullException();
  gbtTreeActionRep *action = dynamic_cast<gbtTreeActionRep *>(p_action.Get());
  if (!action || action->m_infoset->m_player != m_player) {
    throw gbtGameMismatchException();
  }

  for (const gbtTreeSequenceRep *seq = this; seq->m_action; seq = seq->m_parent) {
    if (seq->m_action == action)  return true;
  }

  return false;
}

//======================================================================
//             Implementation of class gbtTreePlayerRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTreePlayerRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreePlayerRep::gbtTreePlayerRep(gbtTreeGameRep *p_efg, int p_id)
  : m_refCount(0), m_id(p_id), m_efg(p_efg), m_deleted(false)
{ }


gbtTreePlayerRep::~gbtTreePlayerRep()
{
  for (int iset = 1; iset <= m_infosets.Length(); 
       m_infosets[iset++]->Delete());
  for (int st = 1; st <= m_strategies.Length(); m_strategies[st++]->Delete());
}

//----------------------------------------------------------------------
//     class gbtTreePlayerRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreePlayerRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_efg->m_refCount++;
}

bool gbtTreePlayerRep::Dereference(void)
{
  if (!m_deleted && --m_efg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the player to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_efg;
  }
  return (--m_refCount == 0 && m_deleted); 
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
  if (!m_efg->m_hasComputed)  m_efg->BuildComputedElements();
  return m_sequences.Length();
}

gbtGameSequence gbtTreePlayerRep::GetSequence(int p_index) const
{
  if (!m_efg->m_hasComputed)  m_efg->BuildComputedElements();
  return m_sequences[p_index];
}

//----------------------------------------------------------------------
//    class gbtTreePlayerRep: Accessing the strategies of the player
//----------------------------------------------------------------------

int gbtTreePlayerRep::NumStrategies(void) const
{
  if (!m_efg->m_hasComputed)  m_efg->BuildComputedElements();
  return m_strategies.Length();
}

gbtGameStrategy gbtTreePlayerRep::GetStrategy(int p_index) const
{
  if (!m_efg->m_hasComputed)  m_efg->BuildComputedElements();
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


