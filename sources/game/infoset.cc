//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of API for information sets
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

#include "game.h"

// Declaration of game API
#include "efg.h"

// Declaration of internal extensive form classes
#include "efgint.h"


//
// This file contains the implementation of:
// (1) The API for information sets and actions for extensive forms;
// (2) The wrapper classes gbtEfgInfoset and gbtEfgAction for these
//     concepts
//
// It seems to make the most sense to place actions in the same place
// as information sets, as actions are not used much independently from
// information sets.
//


//----------------------------------------------------------------------
//              gbtEfgActionBase: Member functions
//----------------------------------------------------------------------

gbtEfgActionBase::gbtEfgActionBase(gbtEfgInfosetBase *p_infoset,
				   int p_id)
  : m_id(p_id), m_infoset(p_infoset), m_deleted(false), 
    m_refCount(0)
{ }

bool gbtEfgActionBase::Precedes(gbtEfgNode n) const
{
  while (n != n->GetGame().GetRoot()) {
    if (n->GetPriorAction().Get() == this) {
      return true;
    }
    else {
      n = n->GetParent();
    }
  }
  return false;
}

gbtNumber gbtEfgActionBase::GetChanceProb(void) const
{
  return m_infoset->m_chanceProbs[m_id];
}

void gbtEfgActionBase::DeleteAction(void)
{
  if (m_infoset->m_actions.Length() == 1) {
    return;
  }

  m_infoset->m_player->m_efg->DeleteAction(m_infoset, this);
}

gbtEfgInfoset gbtEfgActionBase::GetInfoset(void) const { return m_infoset; }

gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgAction &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//           class gbtEfgInfosetBase: Member functions
//----------------------------------------------------------------------

gbtEfgInfosetBase::gbtEfgInfosetBase(gbtEfgPlayerBase *p_player,
				     int p_id, int p_br)
  : m_id(p_id), m_player(p_player), m_deleted(false), 
    m_refCount(0), m_actions(p_br),
    m_chanceProbs((p_player->m_id == 0) ? p_br : 0),
    m_flag(0), m_whichbranch(0)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbtEfgActionBase(this, act);
    if (p_player->m_id == 0) {
      m_chanceProbs[act] = gbtRational(1, p_br);
    }
  }
}

gbtEfgInfosetBase::~gbtEfgInfosetBase()
{
  for (int act = 1; act <= m_actions.Length(); act++) {
    if (m_actions[act]->m_refCount == 0) {
      delete m_actions[act];
    }
    else {
      m_actions[act]->m_deleted = true;
    }
  }
}

void gbtEfgInfosetBase::PrintActions(gbtOutput &f) const
{ 
  f << "{ ";
  for (int i = 1; i <= m_actions.Length(); i++) {
    f << '"' << EscapeQuotes(m_actions[i]->m_label) << "\" ";
    if (m_player->m_id == 0) {
      f << m_chanceProbs[i] << ' ';
    }
  }
  f << "}";
}


void gbtEfgInfosetBase::DeleteInfoset(void)
{
  if (NumMembers() > 0)  {
    return;
  }

  m_player->m_efg->DeleteInfoset(this);
}

gbtEfgAction gbtEfgInfosetBase::GetAction(int p_index) const
{
  return m_actions[p_index];
}

int gbtEfgInfosetBase::NumActions(void) const
{
  return m_actions.Length();
}

gbtEfgNode gbtEfgInfosetBase::GetMember(int p_index) const
{
  return m_members[p_index];
}

int gbtEfgInfosetBase::NumMembers(void) const
{
  return m_members.Length();
}

gbtEfgPlayer gbtEfgInfosetBase::GetPlayer(void) const
{
  return m_player;
}

void gbtEfgInfosetBase::SetPlayer(gbtEfgPlayer p_player)
{
  if (GetPlayer()->IsChance() || p_player->IsChance()) {
    throw gbtEfgbtException();
  }
  
  if (GetPlayer() == p_player) {
    return;
  }

  m_player->m_efg->SetPlayer(this, dynamic_cast<gbtEfgPlayerBase *>(p_player.Get()));
}

bool gbtEfgInfosetBase::IsChanceInfoset(void) const
{
  return (m_player->m_id == 0);
}

gbtEfgGame gbtEfgInfosetBase::GetGame(void) const
{
  return m_player->m_efg;
}

bool gbtEfgInfosetBase::Precedes(gbtEfgNode p_node) const
{
  gbtEfgNodeBase *node = dynamic_cast<gbtEfgNodeBase *>(p_node.Get());
  while (node) {
    if (node->m_infoset == this) {
      return true;
    }
    else {
      node = node->m_parent;
    }
  }
  return false;
}

void gbtEfgInfosetBase::Reveal(gbtEfgPlayer p_who)
{
  m_player->m_efg->Reveal(this,
			  dynamic_cast<gbtEfgPlayerBase *>(p_who.Get()));
}

void gbtEfgInfosetBase::MergeInfoset(gbtEfgInfoset p_from)
{
  gbtEfgInfosetBase *from = dynamic_cast<gbtEfgInfosetBase *>(p_from.Get());
  if (this == from ||
      m_actions.Length() != from->m_actions.Length())  {
    return;
  }

  // FIXME: Can't bridge subgames
  if (m_members[1]->m_gameroot != from->m_members[1]->m_gameroot) {
    return;
  }

  m_player->m_efg->MergeInfoset(this, from);
}

gbtEfgAction gbtEfgInfosetBase::InsertAction(int where)
{
  gbtEfgActionBase *action = new gbtEfgActionBase(this, where);
  m_actions.Insert(action, where);
  if (m_player->m_id == 0) {
    m_chanceProbs.Insert(0, where);
  }
  for (; where <= m_actions.Length(); where++) {
    m_actions[where]->m_id = where;
  }
  return action;
}

void gbtEfgInfosetBase::SetChanceProb(int p_action, const gbtNumber &p_value)
{
  m_chanceProbs[p_action] = p_value;
}

gbtNumber gbtEfgInfosetBase::GetChanceProb(int p_action) const
{
  return m_chanceProbs[p_action];
}

bool gbtEfgInfosetBase::GetFlag(void) const
{ 
  return m_flag;
}

void gbtEfgInfosetBase::SetFlag(bool p_flag)
{
  m_flag = p_flag;
}

int gbtEfgInfosetBase::GetWhichBranch(void) const
{
  return m_whichbranch;
}

void gbtEfgInfosetBase::SetWhichBranch(int p_branch) 
{
  m_whichbranch = p_branch;
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgInfoset &)
{ return p_stream; }
 
