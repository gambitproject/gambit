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

//----------------------------------------------------------------------
//                    class Node: Member functions
//----------------------------------------------------------------------

Node::Node(efgGame *e, Node *p)
  : mark(false), number(0), E(e), infoset(0), parent(p), outcome(0),
    gameroot((p) ? p->gameroot : this)
{ }

Node::~Node()
{
  for (int i = children.Length(); i; delete children[i--]);
}


int Node::NumberInInfoset(void) const
{
  for (int i = 1; i <= GetInfoset().NumMembers(); i++)
    if (GetInfoset().GetMember(i) == this)
      return i;
  //  This could be speeded up by adding a member to Node to keep track of this
  throw efgGame::Exception();
}

gbtEfgInfoset Node::GetInfoset(void) const
{
  return infoset;
}

Node *Node::NextSibling(void) const  
{
  if (!parent)   return 0;
  if (parent->children.Find((Node * const) this) == parent->children.Length())
    return 0;
  else
    return parent->children[parent->children.Find((Node * const)this) + 1];
}

Node *Node::PriorSibling(void) const
{ 
  if (!parent)   return 0;
  if (parent->children.Find((Node * const)this) == 1)
    return 0;
  else
    return parent->children[parent->children.Find((Node * const)this) - 1];

}

gbtEfgAction Node::GetAction(void) const
{
  if (this == GetGame()->RootNode()) {
    return gbtEfgAction();
  }
  
  gbtEfgInfoset infoset = GetParent()->GetInfoset();
  for (int i = 1; i <= infoset.NumActions(); i++) {
    if (this == GetParent()->GetChild(infoset.GetAction(i))) {
      return infoset.GetAction(i);
    }
  }

  return 0;
}

void Node::DeleteOutcome(gbt_efg_outcome_rep *p_outcome)
{
  if (p_outcome == outcome) {
    outcome = 0;
  }
  for (int i = 1; i <= children.Length(); i++) {
    children[i]->DeleteOutcome(p_outcome);
  }
}

gbtEfgPlayer Node::GetPlayer(void) const
{
  if (!infoset) {
    return 0;
  }
  else {
    return infoset->m_player;
  }
}

Node *Node::GetChild(const gbtEfgAction &p_action) const
{
  if (p_action.GetInfoset() != infoset) {
    return 0;
  }
  else {
    return children[p_action.GetId()];
  }
}

gbtEfgOutcome Node::GetOutcome(void) const
{
  return outcome;
}

void Node::SetOutcome(const gbtEfgOutcome &p_outcome)
{
  outcome = p_outcome.rep;
}
