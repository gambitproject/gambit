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
#include "efg.h"

// Declaration of internal extensive form classes
#include "efgint.h"

//
// This file contains the implementation of the API for nodes in
// a game tree.
//

gbt_efg_node_rep::gbt_efg_node_rep(efgGame *p_efg, gbt_efg_node_rep *p_parent)
  : m_id(0), m_efg(p_efg), m_deleted(false), m_refCount(1),
    m_mark(false), m_infoset(0), m_parent(p_parent), m_outcome(0),
    m_whichbranch(0), m_ptr(0),
    m_gameroot((p_parent) ? p_parent->m_gameroot : this)
{ }

gbt_efg_node_rep::~gbt_efg_node_rep()
{
  for (int i = m_children.Length(); i; delete m_children[i--]);
}

void gbt_efg_node_rep::DeleteOutcome(gbt_efg_outcome_rep *p_outcome)
{
  if (p_outcome == m_outcome) {
    m_outcome = 0;
  }
  for (int i = 1; i <= m_children.Length(); i++) {
    m_children[i]->DeleteOutcome(p_outcome);
  }
}

gbtEfgNode::gbtEfgNode(void)
  : rep(0)
{ }

gbtEfgNode::gbtEfgNode(gbt_efg_node_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgNode::gbtEfgNode(const gbtEfgNode &p_action)
  : rep(p_action.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgNode::~gbtEfgNode()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      delete rep;
    }
  }
}

gbtEfgNode &gbtEfgNode::operator=(const gbtEfgNode &p_action)
{
  if (this == &p_action) {
    return *this;
  }

  if (rep && --rep->m_refCount == 0) {
    delete rep;
  }

  if ((rep = p_action.rep) != 0) {
    rep->m_refCount++;
  }
  return *this;
}

bool gbtEfgNode::operator==(const gbtEfgNode &p_action) const
{
  return (rep == p_action.rep);
} 

bool gbtEfgNode::operator!=(const gbtEfgNode &p_action) const
{
  return (rep != p_action.rep);
} 

bool gbtEfgNode::IsNull(void) const
{
  return (rep == 0);
}

int gbtEfgNode::GetId(void) const
{
  return (rep) ? rep->m_id : -1;
}

gText gbtEfgNode::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtEfgNode::SetLabel(const gText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

gbtEfgInfoset gbtEfgNode::GetInfoset(void) const
{
  if (rep) {
    return rep->m_infoset;
  }
  else {
    return 0;
  }
}

efgGame *gbtEfgNode::GetGame(void) const
{
  if (rep) {
    return rep->m_efg;
  }
  else {
    return 0;
  }
}

gbtEfgNode gbtEfgNode::GetParent(void) const
{
  if (rep) {
    return rep->m_parent;
  }
  else {
    return 0;
  }
}

gbtEfgNode gbtEfgNode::GetSubgameRoot(void) const
{
  if (rep) {
    return rep->m_gameroot;
  }
  else {
    return 0;
  }
}

gbtEfgNode gbtEfgNode::GetChild(const gbtEfgAction &p_action) const
{
  if (!rep || p_action.GetInfoset() != rep->m_infoset) {
    return 0;
  }
  else {
    return rep->m_children[p_action.GetId()];
  }
}

gbtEfgNode gbtEfgNode::GetChild(int p_child) const
{
  if (rep) {
    return rep->m_children[p_child];
  }
  else {
    return 0;
  }
}

int gbtEfgNode::NumChildren(void) const
{
  if (rep) {
    return rep->m_children.Length();
  }
  else {
    return 0;
  }
}

gbtEfgOutcome gbtEfgNode::GetOutcome(void) const
{
  if (rep) {
    return rep->m_outcome;
  }
  else {
    return 0;
  }
}

void gbtEfgNode::SetOutcome(const gbtEfgOutcome &p_outcome)
{
  if (rep) {
    rep->m_outcome = p_outcome.rep;
  }
}

int gbtEfgNode::NumberInInfoset(void) const
{
  for (int i = 1; i <= GetInfoset().NumMembers(); i++) {
    if (GetInfoset().GetMember(i) == *this) {
      return i;
    }
  }
  //  This could be sped up by adding a member to keep track of this
  throw efgGame::Exception();
}

gbtEfgNode gbtEfgNode::NextSibling(void) const  
{
  if (!rep || !rep->m_parent)   return 0;
  if (rep->m_parent->m_children.Find(rep) == rep->m_parent->m_children.Length())
    return 0;
  else
    return rep->m_parent->m_children[rep->m_parent->m_children.Find(rep) + 1];
}

gbtEfgNode gbtEfgNode::PriorSibling(void) const
{ 
  if (!rep || !rep->m_parent)   return 0;
  if (!rep->m_parent->m_children.Find(rep) == 1)
    return 0;
  else
    return rep->m_parent->m_children[rep->m_parent->m_children.Find(rep) - 1];

}

gbtEfgAction gbtEfgNode::GetAction(void) const
{
  if (*this == GetGame()->RootNode()) {
    return gbtEfgAction();
  }
  
  gbtEfgInfoset infoset = GetParent().GetInfoset();
  for (int i = 1; i <= infoset.NumActions(); i++) {
    if (*this == GetParent().GetChild(infoset.GetAction(i))) {
      return infoset.GetAction(i);
    }
  }

  return 0;
}


gbtEfgPlayer gbtEfgNode::GetPlayer(void) const
{
  if (!rep || !rep->m_infoset) {
    return 0;
  }
  else {
    return rep->m_infoset->m_player;
  }
}

bool gbtEfgNode::IsPredecessor(const gbtEfgNode &p_of) const
{
  gbt_efg_node_rep *n;
  for (n = p_of.rep; n && n != rep; n = n->m_parent);
  return (n == rep);
}

gOutput &operator<<(gOutput &p_stream, const gbtEfgNode &)
{ return p_stream; }

