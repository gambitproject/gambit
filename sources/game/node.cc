//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of API for nodes
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
#include "nfgsupport.h"

#include "gamebase.h"

//
// This file contains the implementation of the API for nodes in
// a game tree.
//

gbtGameNodeBase::gbtGameNodeBase(gbtGameBase *p_efg,
			       gbtGameNodeBase *p_parent)
  : m_id(0), m_efg(p_efg), m_deleted(false), m_refCount(0),
    m_mark(false), m_infoset(0), m_parent(p_parent), m_outcome(0),
    m_whichbranch(0), m_ptr(0),
    m_gameroot((p_parent) ? p_parent->m_gameroot : this)
{ }

gbtGameNodeBase::~gbtGameNodeBase()
{
  // for (int i = m_children.Length(); i; delete m_children[i--]);
}

void gbtGameNodeBase::DeleteOutcome(gbtGameOutcomeBase *p_outcome)
{
  if (p_outcome == m_outcome) {
    m_outcome = 0;
  }
  for (int i = 1; i <= m_children.Length(); i++) {
    m_children[i]->DeleteOutcome(p_outcome);
  }
}

gbtGameNode gbtGameNodeBase::GetNextSibling(void) const
{
  if (!m_parent) {
    return 0;
  }

  int childNumber = m_parent->m_children.Find(const_cast<gbtGameNodeBase *>(this));
  if (childNumber == m_parent->m_children.Length()) {
    return 0;
  }
  else {
    return m_parent->m_children[childNumber + 1];
  }
}

gbtGameNode gbtGameNodeBase::GetPriorSibling(void) const
{
  if (!m_parent) {
    return 0;
  }

  int childNumber = m_parent->m_children.Find(const_cast<gbtGameNodeBase *>(this));
  if (childNumber == 1) {
    return 0;
  }
  else {
    return m_parent->m_children[childNumber - 1];
  }
}

gbtGameNode gbtGameNodeBase::GetNextMember(void) const
{
  if (!m_infoset) {
    return 0;
  }

  int memberNumber = m_infoset->m_members.Find(const_cast<gbtGameNodeBase *>(this));
  if (memberNumber == m_infoset->m_members.Length()) {
    return 0;
  }
  else {
    return m_infoset->m_members[memberNumber + 1];
  }
}

gbtGameNode gbtGameNodeBase::GetPriorMember(void) const
{
  if (!m_infoset) {
    return 0;
  }

  int memberNumber = m_infoset->m_members.Find(const_cast<gbtGameNodeBase *>(this));
  if (memberNumber == 1) {
    return 0;
  }
  else {
    return m_infoset->m_members[memberNumber - 1];
  }
}

//
// MarkSubtree: sets the Node::mark flag on all children of node
//
void gbtGameNodeBase::MarkSubtree(bool p_mark)
{
  m_mark = p_mark;
  for (int i = 1; i <= m_children.Length(); i++) {
    m_children[i]->MarkSubtree(p_mark);
  }
}

gbtGameInfoset gbtGameNodeBase::GetInfoset(void) const
{
  return m_infoset;
}

gbtGame gbtGameNodeBase::GetGame(void) const
{
  return m_efg;
}

gbtGameNode gbtGameNodeBase::GetParent(void) const
{
  return m_parent;
}

gbtGameNode gbtGameNodeBase::GetSubgameRoot(void) const
{
  return m_gameroot;
}

gbtGameNode gbtGameNodeBase::GetChild(const gbtGameAction &p_action) const
{
  if (p_action->GetInfoset() != GetInfoset()) {
    return 0;
  }
  else {
    return m_children[p_action->GetId()];
  }
}

gbtGameNode gbtGameNodeBase::GetChild(int p_child) const
{
  return m_children[p_child];
}

int gbtGameNodeBase::NumChildren(void) const
{
  return m_children.Length();
}

gbtGameOutcome gbtGameNodeBase::GetOutcome(void) const
{
  return m_outcome;
}

void gbtGameNodeBase::SetOutcome(const gbtGameOutcome &p_outcome)
{
  m_outcome = dynamic_cast<gbtGameOutcomeBase *>(p_outcome.Get());
}

int gbtGameNodeBase::GetMemberId(void) const
{
  if (!m_infoset) {
    return 0;
  }

  for (int i = 1; i <= GetInfoset()->NumMembers(); i++) {
    if (m_infoset->m_members[i] == this) {
      return i;
    }
  }

  return 0;
}

gbtGameAction gbtGameNodeBase::GetPriorAction(void) const
{
  if (this == m_efg->root) {
    return gbtGameAction();
  }
  
  gbtGameInfosetBase *infoset = m_parent->m_infoset;
  for (int i = 1; i <= infoset->m_actions.Length(); i++) {
    if (this == m_parent->m_children[i]) {
      return infoset->m_actions[i];
    }
  }

  return 0;
}


gbtGamePlayer gbtGameNodeBase::GetPlayer(void) const
{
  if (!m_infoset) {
    return 0;
  }
  else {
    return m_infoset->m_player;
  }
}

bool gbtGameNodeBase::IsPredecessorOf(const gbtGameNode &p_node) const
{
  gbtGameNodeBase *n = dynamic_cast<gbtGameNodeBase *>(p_node.Get());
  for (; n && n != this; n = n->m_parent);
  return (n == this);
}

bool gbtGameNodeBase::IsSubgameRoot(void) const
{
  if (NumChildren() == 0) {
    return false;
  }

  m_efg->MarkTree(this, this);
  return m_efg->CheckTree(this, this);
}

gbtGameNode gbtGameNodeBase::InsertMove(gbtGameInfoset p_infoset)
{
  if (p_infoset.IsNull()) {
    throw gbtGameException();
  }

  gbtGameInfosetBase *infoset = dynamic_cast<gbtGameInfosetBase *>(p_infoset.Get());
  // FIXME: For the moment, can't bridge subgames
  if (infoset->m_members.Length() > 0 &&
      m_gameroot != infoset->m_members[1]->m_gameroot) {
    return 0;
  }  

  m_efg->InsertMove(this, infoset);
  return GetParent();
}

void gbtGameNodeBase::DeleteMove(void)
{
  if (GetParent().IsNull()) {
    return;
  }

  // FIXME: Unmarking all subgames to be conservative.
  // Is this necessary?  (Or, more likely, will be moot once subgame
  // implementation is improved!)
  gbtGame(m_efg)->UnmarkSubgames(gbtGameNode(this));
  m_efg->DeleteMove(this);
}

void gbtGameNodeBase::DeleteTree(void)
{
  m_efg->sortisets = false;
  m_efg->DeleteTree(this);
  m_efg->sortisets = true;
}

void gbtGameNodeBase::JoinInfoset(gbtGameInfoset p_infoset)
{
  if (p_infoset.IsNull())  {
    throw gbtGameException();
  }

  gbtGameInfosetBase *infoset = dynamic_cast<gbtGameInfosetBase *>(p_infoset.Get());
  // FIXME: can't bridge subgames
  if (infoset->m_members.Length() > 0 &&
      m_gameroot != infoset->m_members[1]->m_gameroot) {
    return;
  }
  
  if (!m_infoset ||
      m_infoset == infoset ||
      infoset->m_actions.Length() != m_children.Length())  {
    return;
  }

  m_efg->JoinInfoset(infoset, this);
}

gbtGameInfoset gbtGameNodeBase::LeaveInfoset(void)
{
  if (!m_infoset) {
    return 0;
  }

  return m_efg->LeaveInfoset(this);
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtGameNode &)
{ return p_stream; }
