//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of nodes in explicit game trees
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
//              Implementation of class gbtTreeNodeRep
//======================================================================

//----------------------------------------------------------------------
//         class gbtTreeNodeRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeNodeRep::gbtTreeNodeRep(gbtTreeGameRep *p_efg,
			       gbtTreeNodeRep *p_parent)
  : m_refCount(0), m_id(0), m_efg(p_efg), m_deleted(false),
    m_infoset(0), m_parent(p_parent), m_outcome(0),
    m_whichbranch(0), m_ptr(0)
{ }

gbtTreeNodeRep::~gbtTreeNodeRep()
{
  // for (int i = m_children.Length(); i; delete m_children[i--]);
}

//----------------------------------------------------------------------
//      class gbtTreeNodeRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeNodeRep::Reference(void)
{
  m_refCount++;
  if (!m_deleted) m_efg->m_refCount++;
}

bool gbtTreeNodeRep::Dereference(void)
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
//      class gbtTreeNodeRep: General information about the node
//----------------------------------------------------------------------

int gbtTreeNodeRep::GetId(void) const
{ return m_id; }

std::string gbtTreeNodeRep::GetLabel(void) const
{ return m_label; }

void gbtTreeNodeRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

//----------------------------------------------------------------------
//       class gbtTreeNodeRep: Accessing relatives of the node
//----------------------------------------------------------------------

int gbtTreeNodeRep::NumChildren(void) const
{ return m_children.Length(); }

bool gbtTreeNodeRep::IsTerminal(void) const
{ return (NumChildren() == 0); }

bool gbtTreeNodeRep::IsNonterminal(void) const
{ return (NumChildren() != 0); }

gbtGameNode gbtTreeNodeRep::GetChild(const gbtGameAction &p_action) const
{
  if (p_action.IsNull())  throw gbtGameNullException();

  gbtTreeActionRep *action = dynamic_cast<gbtTreeActionRep *>(p_action.Get());
  if (!action || action->m_infoset != m_infoset) {
    throw gbtGameMismatchException();
  }

  return m_children[action->m_id];
}

gbtGameNode gbtTreeNodeRep::GetChild(int p_child) const
{ return m_children[p_child]; }

bool gbtTreeNodeRep::IsPredecessorOf(const gbtGameNode &p_node) const
{
  if (p_node.IsNull())  throw gbtGameNullException();
  
  gbtTreeNodeRep *n = dynamic_cast<gbtTreeNodeRep *>(p_node.Get());
  if (!n || n->m_efg != m_efg)  throw gbtGameMismatchException();
  for (; n && n != this; n = n->m_parent);
  return (n == this);
}

gbtGameNode gbtTreeNodeRep::GetParent(void) const
{ return m_parent; }

gbtGameAction gbtTreeNodeRep::GetPriorAction(void) const
{
  if (!m_parent)  return gbtGameAction();
  return m_parent->m_infoset->m_actions[m_parent->m_children.Find(const_cast<gbtTreeNodeRep *>(this))];
}

gbtGameNode gbtTreeNodeRep::GetPriorSibling(void) const
{
  if (!m_parent)  return 0;
  int index = m_parent->m_children.Find(const_cast<gbtTreeNodeRep *>(this));
  if (index == 1) return 0;
  else return m_parent->m_children[index - 1];
}

gbtGameNode gbtTreeNodeRep::GetNextSibling(void) const  
{
  if (!m_parent)   return 0;
  int index = m_parent->m_children.Find(const_cast<gbtTreeNodeRep *>(this));
  if (index == m_parent->m_children.Length()) return 0;
  else return m_parent->m_children[index + 1];
}

//----------------------------------------------------------------------
//   class gbtTreeNodeRep: Accessing the information set of the node
//----------------------------------------------------------------------

gbtGameInfoset gbtTreeNodeRep::GetInfoset(void) const
{ return m_infoset; }

int gbtTreeNodeRep::GetMemberId(void) const
{
  if (!m_infoset) return 0;
  return m_infoset->m_members.Find(const_cast<gbtTreeNodeRep *>(this));
}

gbtGameNode gbtTreeNodeRep::GetPriorMember(void) const
{
  if (!m_infoset) return 0;
  int index = m_infoset->m_members.Find(const_cast<gbtTreeNodeRep *>(this));
  if (index == 1) return 0;
  else return m_infoset->m_members[index - 1];
}

gbtGameNode gbtTreeNodeRep::GetNextMember(void) const
{
  if (!m_infoset)  return 0;
  int index = m_infoset->m_members.Find(const_cast<gbtTreeNodeRep *>(this));
  if (index == m_infoset->m_members.Length()) return 0;
  else return m_infoset->m_members[index + 1];
}

gbtGamePlayer gbtTreeNodeRep::GetPlayer(void) const
{
  return (m_infoset) ? m_infoset->m_player : 0;
}

//----------------------------------------------------------------------
//     class gbtTreeNodeRep: Accessing the outcome at a node
//----------------------------------------------------------------------

void gbtTreeNodeRep::SetOutcome(const gbtGameOutcome &p_outcome)
{
  if (p_outcome.IsNull()) {
    m_outcome = 0;
    return;
  }

  gbtTreeOutcomeRep *outcome = dynamic_cast<gbtTreeOutcomeRep *>(p_outcome.Get());
  if (!outcome || outcome->m_efg != m_efg)  throw gbtGameMismatchException();
  m_outcome = dynamic_cast<gbtTreeOutcomeRep *>(p_outcome.Get());
}

gbtGameOutcome gbtTreeNodeRep::GetOutcome(void) const
{ return m_outcome; }

//----------------------------------------------------------------------
//           class gbtTreeNodeRep: Editing the game tree
//----------------------------------------------------------------------

gbtGameNode gbtTreeNodeRep::InsertMove(gbtGameInfoset p_infoset)
{
  if (p_infoset.IsNull())  throw gbtGameNullException();

  gbtTreeInfosetRep *infoset = dynamic_cast<gbtTreeInfosetRep *>(p_infoset.Get());
  if (!infoset || infoset->m_player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }

  gbtTreeNodeRep *node = new gbtTreeNodeRep(m_efg, m_parent);
  node->m_infoset = infoset;
  infoset->m_members.Append(node);
  if (m_parent) {
    m_parent->m_children[m_parent->m_children.Find(this)] = node;
  }
  else {
    m_efg->m_root = node;
  }
  node->m_children.Append(this);
  m_parent = node;
  for (int i = infoset->m_actions.Length() - 1; i; i--) {
    node->m_children.Append(new gbtTreeNodeRep(m_efg, node));
  }

  m_efg->OnStrategiesChanged();
  m_efg->SortInfosets();

  return m_parent;
}

//!
//! Deletes the node which is the parent of this node; this node
//! then assumes its parent's place in the tree.  All subtrees
//! descended from the parnet node are also deleted.
//!
void gbtTreeNodeRep::DeleteMove(void)
{
  if (!m_parent) return;

  // turn infoset sorting off during tree deletion 
  m_efg->m_sortInfosets = false;

  m_parent->m_children.Remove(m_parent->m_children.Find(this));
  m_parent->DeleteTree();
  m_parent = m_parent->m_parent;
  if (m_parent->m_parent) {
    m_parent->m_parent->m_children[m_parent->m_parent->m_children.Find(m_parent)] = this;
  }
  else {
    m_efg->m_root = this;
  }

  m_parent->m_deleted = true;

  m_efg->OnStrategiesChanged();
  m_efg->m_sortInfosets = true;
  m_efg->SortInfosets();
}

//!
//! This auxiliary function does the recursive business of deleting
//! a tree.
//!
void gbtTreeNodeRep::DeleteSubtree(void)
{
  while (m_children.Length() > 0)   {
    gbtTreeNodeRep *child = m_children.Remove(1);
    child->DeleteSubtree();
    child->m_deleted = true;
  }

  if (m_infoset)  {
    m_infoset->m_members.Remove(m_infoset->m_members.Find(this));
    m_infoset = 0;
  }
  m_outcome = 0;
  m_label = "";
}

void gbtTreeNodeRep::DeleteTree(void)
{
  m_efg->m_sortInfosets = false;
  DeleteSubtree();
  m_efg->m_sortInfosets = true;
  m_efg->OnStrategiesChanged();
  m_efg->SortInfosets();
}

//!
//! This auxiliary function does the recursive business of copying a
//! subtree.  The parameter stop helps handle the case where the copy
//! is specified from a node to a descendant; in this case, the copy
//! operation stops at the descendant node (to avoid an infinite loop).
//!
void gbtTreeNodeRep::CopySubtree(gbtTreeNodeRep *dest, gbtTreeNodeRep *stop)
{
  if (this == stop) {
    dest->m_outcome = m_outcome;
    return;
  }

  if (m_children.Length())  {
    dest->InsertMove(m_infoset);
    for (int i = 1; i <= m_children.Length(); i++) {
      m_children[i]->CopySubtree(m_parent->m_children[i], stop);
    }
  }

  dest->m_label = m_label;
  dest->m_outcome = m_outcome;
}

gbtGameNode gbtTreeNodeRep::CopyTree(gbtGameNode p_src)
{
  if (p_src.IsNull())  throw gbtGameNullException();

  gbtTreeNodeRep *src = dynamic_cast<gbtTreeNodeRep *>(p_src.Get());
  if (!src || m_children.Length() > 0)  throw gbtGameMismatchException();

  gbtTreeNodeRep *dest = this;
  if (src == dest)  return src;

  if (src->m_children.Length())  {
    dest->InsertMove(src->m_infoset);
    for (int i = 1; i <= src->m_children.Length(); i++) {
      src->m_children[i]->CopySubtree(dest->m_parent->m_children[i],
				      dest->m_parent);
    }
    
    m_efg->OnStrategiesChanged();
    m_efg->SortInfosets();
  }

  return dest;
}

gbtGameNode gbtTreeNodeRep::MoveTree(gbtGameNode p_src)
{
  if (p_src.IsNull())  throw gbtGameNullException();

  gbtTreeNodeRep *src = dynamic_cast<gbtTreeNodeRep *>(p_src.Get());
  if (!src || m_children.Length() > 0)  throw gbtGameMismatchException();

  gbtTreeNodeRep *dest = this;

  if (src == dest || src->IsPredecessorOf(dest)) return src;

  if (src->m_parent == dest->m_parent) {
    int srcChild = src->m_parent->m_children.Find(src);
    int destChild = src->m_parent->m_children.Find(dest);
    src->m_parent->m_children[srcChild] = dest;
    src->m_parent->m_children[destChild] = src;
  }
  else {
    gbtTreeNodeRep *parent = src->m_parent; 
    parent->m_children[parent->m_children.Find(src)] = dest;
    dest->m_parent->m_children[dest->m_parent->m_children.Find(dest)] = src;
    src->m_parent = dest->m_parent;
    dest->m_parent = parent;
  }

  dest->m_label = "";
  dest->m_outcome = 0;
  
  m_efg->OnStrategiesChanged();
  m_efg->SortInfosets();
  return dest;
}

void gbtTreeNodeRep::JoinInfoset(const gbtGameInfoset &p_infoset)
{
  if (p_infoset.IsNull())  throw gbtGameNullException();

  gbtTreeInfosetRep *infoset = dynamic_cast<gbtTreeInfosetRep *>(p_infoset.Get());
  if (!infoset || infoset->NumActions() != m_children.Length()) {
    throw gbtGameMismatchException();
  }

  if (infoset == m_infoset) return;

  m_infoset->m_members.Remove(m_infoset->m_members.Find(this));
  infoset->m_members.Append(this);
  m_infoset = infoset;

  m_efg->OnStrategiesChanged();
  m_efg->SortInfosets();
}


gbtGameInfoset gbtTreeNodeRep::LeaveInfoset(void)
{
  if (!m_infoset)   return 0;
  if (m_infoset->m_members.Length() == 1)  return m_infoset;

  gbtTreeInfosetRep *oldInfoset = m_infoset;
  gbtTreePlayerRep *p = m_infoset->m_player;
  m_infoset->m_members.Remove(m_infoset->m_members.Find(this));
  m_infoset = dynamic_cast<gbtTreeInfosetRep *>(p->NewInfoset(m_children.Length()).Get());
  m_infoset->m_label = oldInfoset->m_label;
  m_infoset->m_members.Append(this);
  for (int i = 1; i <= oldInfoset->m_actions.Length(); i++) {
    m_infoset->m_actions[i]->m_label = oldInfoset->m_actions[i]->m_label;
  }

  m_efg->OnStrategiesChanged();
  m_efg->SortInfosets();
  return m_infoset;
}

//----------------------------------------------------------------------
//             class gbtTreeNodeRep: Internal members
//----------------------------------------------------------------------

//!
//! Recursively unassigns the outcome from all nodes in the subtree
//! rooted at the node.
//!
void gbtTreeNodeRep::DeleteOutcome(gbtTreeOutcomeRep *p_outcome)
{
  if (p_outcome == m_outcome) {
    m_outcome = 0;
  }
  for (int i = 1; i <= m_children.Length(); i++) {
    m_children[i]->DeleteOutcome(p_outcome);
  }
}

