//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of information sets for explicit game trees
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

// Declaration of game API
#include "game.h"

// Declaration of internal extensive form classes
#include "tree-game.h"


//======================================================================
//             Implementation of class gbtTreeActionRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTreeActionRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeActionRep::gbtTreeActionRep(gbtTreeInfosetRep *p_infoset, int p_id)
  : m_refCount(0), m_id(p_id), m_infoset(p_infoset), m_deleted(false)
{ }

//----------------------------------------------------------------------
//     class gbtTreeActionRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeActionRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_infoset->m_player->m_efg->m_refCount++;
}

bool gbtTreeActionRep::Dereference(void)
{
  if (!m_deleted && --m_infoset->m_player->m_efg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the player to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_infoset->m_player->m_efg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//    class gbtTreeActionRep: General information about the action
//----------------------------------------------------------------------

int gbtTreeActionRep::GetId(void) const
{ return m_id; }

void gbtTreeActionRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTreeActionRep::GetLabel(void) const
{ return m_label; }

//----------------------------------------------------------------------
//     class gbtTreeActionRep: Information about the game tree
//----------------------------------------------------------------------

gbtGameInfoset gbtTreeActionRep::GetInfoset(void) const
{ return m_infoset; }

bool gbtTreeActionRep::Precedes(const gbtGameNode &p_node) const
{
  gbtTreeNodeRep *node = dynamic_cast<gbtTreeNodeRep *>(p_node.Get());
  if (!node || node->m_efg != m_infoset->m_player->m_efg) {
    throw gbtGameMismatchException();
  }

  while (node->m_parent) {
    gbtTreeNodeRep *parent = node->m_parent;
    if (parent->m_infoset == m_infoset &&
	parent->m_children.Find(node) == m_id) {
      return true;
    }
    node = parent;
  }
  return false;
}

//----------------------------------------------------------------------
//       class gbtTreeActionRep: Manipulating chance actions 
//----------------------------------------------------------------------

gbtRational gbtTreeActionRep::GetChanceProb(void) const
{
  if (m_infoset->m_player->m_id != 0) throw gbtGameMismatchException();
  return m_infoset->m_chanceProbs[m_id];
}

void gbtTreeActionRep::SetChanceProb(const gbtRational &p_prob)
{
  if (m_infoset->m_player->m_id != 0) throw gbtGameMismatchException();
  m_infoset->m_chanceProbs[m_id] = p_prob;
}

//----------------------------------------------------------------------
//           class gbtTreeActionRep: Editing the game tree
//----------------------------------------------------------------------

void gbtTreeActionRep::DeleteAction(void)
{
  if (m_infoset->m_actions.Length() == 1) return;

  m_infoset->m_actions.Remove(m_id);
  if (m_infoset->m_player->m_id == 0) {
    m_infoset->m_chanceProbs.Remove(m_id);
  }
  for (int act = 1; act <= m_infoset->m_actions.Length(); act++) {
    m_infoset->m_actions[act]->m_id = act;
  }

  for (int i = 1; i <= m_infoset->m_members.Length(); i++)   {
    m_infoset->m_members[i]->m_children[m_id]->DeleteTree();
    gbtTreeNodeRep *node = 
      m_infoset->m_members[i]->m_children.Remove(m_id);
    node->Delete();
  }

  m_deleted = true;

  m_infoset->m_player->m_efg->OnStrategiesChanged();
}

//======================================================================
//            Implementation of class gbtTreeInfosetRep
//======================================================================

//----------------------------------------------------------------------
//       class gbtTreeInfosetRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeInfosetRep::gbtTreeInfosetRep(gbtTreePlayerRep *p_player,
				     int p_id, int p_br)
  : m_refCount(0), m_id(p_id), m_player(p_player), 
    m_deleted(false), m_actions(p_br),
    m_chanceProbs((p_player->m_id == 0) ? p_br : 0),
    m_flag(0), m_whichbranch(0)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbtTreeActionRep(this, act);
    if (p_player->m_id == 0) {
      m_chanceProbs[act] = gbtRational(1, p_br);
    }
  }
}

gbtTreeInfosetRep::~gbtTreeInfosetRep()
{
  for (int act = 1; act <= m_actions.Length(); m_actions[act++]->Delete());
}

//----------------------------------------------------------------------
//     class gbtTreeInfosetRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeInfosetRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_player->m_efg->m_refCount++;
}

bool gbtTreeInfosetRep::Dereference(void)
{
  if (!m_deleted && --m_player->m_efg->m_refCount == 0) {
    // Note that as a side effect, deleting the game will cause
    // the player to be marked as deleted (since by definition,
    // at this point the reference count must be at least one)
    delete m_player->m_efg;
  }
  return (--m_refCount == 0 && m_deleted); 
}

//----------------------------------------------------------------------
//     class gbtTreeInfosetRep: General information about the node
//----------------------------------------------------------------------

int gbtTreeInfosetRep::GetId(void) const
{ return m_id; }

void gbtTreeInfosetRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTreeInfosetRep::GetLabel(void) const
{ return m_label; }

//----------------------------------------------------------------------
//   class gbtTreeInfosetRep: Accessing information about the player
//----------------------------------------------------------------------

bool gbtTreeInfosetRep::IsChanceInfoset(void) const
{ return (m_player->m_id == 0); }

gbtGamePlayer gbtTreeInfosetRep::GetPlayer(void) const
{ return m_player; }

void gbtTreeInfosetRep::SetPlayer(const gbtGamePlayer &p_player)
{
  if (p_player.IsNull())  throw gbtGameNullException();
  if (GetPlayer()->IsChance() || p_player->IsChance()) {
    throw gbtGameMismatchException();
  }
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player)  throw gbtGameMismatchException();
  
  if (m_player == player) return;   // nothing to do!

  m_player->m_infosets.Remove(m_player->m_infosets.Find(this));
  m_player = player;
  m_player->m_infosets.Append(this);

  m_player->m_efg->OnStrategiesChanged();
}

//----------------------------------------------------------------------
//   class gbtTreeInfosetRep: Accessing information about the actions
//----------------------------------------------------------------------

gbtGameAction gbtTreeInfosetRep::GetAction(int p_index) const
{ return m_actions[p_index]; }

int gbtTreeInfosetRep::NumActions(void) const
{ return m_actions.Length(); }

//----------------------------------------------------------------------
//   class gbtTreeInfosetRep: Accessing information about the members
//----------------------------------------------------------------------

int gbtTreeInfosetRep::NumMembers(void) const
{ return m_members.Length(); }

gbtGameNode gbtTreeInfosetRep::GetMember(int p_index) const
{ return m_members[p_index]; }

//----------------------------------------------------------------------
//         class gbtTreeInfosetRep: Editing the game tree
//----------------------------------------------------------------------

bool gbtTreeInfosetRep::Precedes(const gbtGameNode &p_node) const
{
  gbtTreeNodeRep *node = dynamic_cast<gbtTreeNodeRep *>(p_node.Get());
  if (!node || node->m_efg != m_player->m_efg) {
    throw gbtGameMismatchException();
  }

  while (node->m_parent) {
    if (node->m_infoset == this) return true;
    else                         node = node->m_parent;
  }
  return false;
}

void gbtTreeInfosetRep::MergeInfoset(const gbtGameInfoset &p_from)
{
  if (p_from.IsNull())  throw gbtGameNullException();
  gbtTreeInfosetRep *from = dynamic_cast<gbtTreeInfosetRep *>(p_from.Get());
  if (!from || m_actions.Length() != from->m_actions.Length()) {
    throw gbtGameMismatchException();
  }

  if (this == from)  return;    // nothing to do

  m_members += from->m_members;
  for (int i = 1; i <= from->m_members.Length(); i++) {
    from->m_members[i]->m_infoset = this;
  }

  from->m_members.Flush();

  m_player->m_efg->OnStrategiesChanged();
}

void gbtTreeInfosetRep::DeleteInfoset(void)
{
  if (NumMembers() > 0)  return;

  m_player->m_infosets.Remove(m_player->m_infosets.Find(this));
  m_deleted = true;
  
  m_player->m_efg->OnStrategiesChanged();
}

gbtGameAction gbtTreeInfosetRep::InsertAction(int where)
{
  gbtTreeActionRep *action = new gbtTreeActionRep(this, where);
  m_actions.Insert(action, where);
  if (m_player->m_id == 0) {
    m_chanceProbs.Insert(0, where);
  }
  for (; where <= m_actions.Length(); where++) {
    m_actions[where]->m_id = where;
  }
  for (int i = 1; i <= m_members.Length(); i++) {
    m_members[i]->m_children.Insert(new gbtTreeNodeRep(m_player->m_efg,
						       m_members[i]),
				    where);

  }
  m_player->m_efg->OnStrategiesChanged();
  return action;
}

void gbtTreeInfosetRep::Reveal(const gbtGamePlayer &p_player)
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_player->m_efg) {
    throw gbtGameMismatchException();
  }

  if (m_actions.Length() <= 1)  return; // only one action; nothing to reveal!

  for (int i = 1; i <= m_actions.Length(); i++) {
    for (int k = 1; k <= player->m_infosets.Length(); k++) {
      // iterate over each member of information set 'k'
      // make copy of members to iterate correctly 
      // (since the information set may be changed in the process)
      gbtArray<gbtTreeNodeRep *> members = player->m_infosets[k]->m_members;
      gbtGameInfoset newiset;
      
      for (int m = 1; m <= members.Length(); m++) {
	gbtTreeNodeRep *n = members[m];
	if (m_actions[i]->Precedes(n)) {
	  if (newiset.IsNull()) {
	    newiset = n->LeaveInfoset();
	  }
	  else {
	    n->JoinInfoset(newiset);
	  }
	} 
      }
    }
  }

  m_player->m_efg->OnStrategiesChanged();
}

