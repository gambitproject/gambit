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

// Declaration of game API
#include "game.h"
#include "nfgsupport.h"

// Declaration of internal game form classes
#include "gamebase.h"


//
// This file contains the implementation of:
// (1) The API for information sets and actions for extensive forms;
// (2) The wrapper classes gbtGameInfoset and gbtGameAction for these
//     concepts
//
// It seems to make the most sense to place actions in the same place
// as information sets, as actions are not used much independently from
// information sets.
//


//----------------------------------------------------------------------
//              gbtGameActionBase: Member functions
//----------------------------------------------------------------------

gbtGameActionBase::gbtGameActionBase(gbtGameInfosetBase *p_infoset,
				   int p_id)
  : m_id(p_id), m_infoset(p_infoset), m_deleted(false)
{ }

bool gbtGameActionBase::Precedes(gbtGameNode n) const
{
  while (!n->IsRoot()) {
    if (n->GetPriorAction().Get() == const_cast<gbtGameActionBase *>(this)) {
      return true;
    }
    else {
      n = n->GetParent();
    }
  }
  return false;
}

gbtNumber gbtGameActionBase::GetChanceProb(void) const
{
  return m_infoset->m_chanceProbs[m_id];
}

void gbtGameActionBase::DeleteAction(void)
{
  if (m_infoset->m_actions.Length() == 1) {
    return;
  }

  m_infoset->m_player->m_efg->DeleteAction(m_infoset, this);
}

gbtGameInfoset gbtGameActionBase::GetInfoset(void) const { return m_infoset; }

gbtOutput &operator<<(gbtOutput &p_stream, const gbtGameAction &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//           class gbtGameInfosetBase: Member functions
//----------------------------------------------------------------------

gbtGameInfosetBase::gbtGameInfosetBase(gbtGamePlayerBase *p_player,
				     int p_id, int p_br)
  : m_id(p_id), m_player(p_player), m_deleted(false), 
    m_refCount(0), m_actions(p_br),
    m_chanceProbs((p_player->m_id == 0) ? p_br : 0),
    m_flag(false), m_whichbranch(0)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbtGameActionBase(this, act);
    if (p_player->m_id == 0) {
      m_chanceProbs[act] = gbtRational(1, p_br);
    }
  }
}

gbtGameInfosetBase::~gbtGameInfosetBase()
{
  /*
  for (int act = 1; act <= m_actions.Length(); act++) {
    if (m_actions[act]->m_refCount == 0) {
      delete m_actions[act];
    }
    else {
      m_actions[act]->m_deleted = true;
    }
  }
  */
}

void gbtGameInfosetBase::PrintActions(gbtOutput &f) const
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


void gbtGameInfosetBase::DeleteInfoset(void)
{
  if (NumMembers() > 0)  {
    return;
  }

  m_player->m_efg->DeleteInfoset(this);
}

gbtGameAction gbtGameInfosetBase::GetAction(int p_index) const
{
  return m_actions[p_index];
}

int gbtGameInfosetBase::NumActions(void) const
{
  return m_actions.Length();
}

gbtGameNode gbtGameInfosetBase::GetMember(int p_index) const
{
  return m_members[p_index];
}

int gbtGameInfosetBase::NumMembers(void) const
{
  return m_members.Length();
}

gbtGamePlayer gbtGameInfosetBase::GetPlayer(void) const
{
  return m_player;
}

void gbtGameInfosetBase::SetPlayer(gbtGamePlayer p_player)
{
  if (GetPlayer()->IsChance() || p_player->IsChance()) {
    throw gbtGameException();
  }
  
  if (GetPlayer() == p_player) {
    return;
  }

  m_player->m_efg->SetPlayer(this, dynamic_cast<gbtGamePlayerBase *>(p_player.Get()));
}

bool gbtGameInfosetBase::IsChanceInfoset(void) const
{
  return (m_player->m_id == 0);
}

bool gbtGameInfosetBase::Precedes(gbtGameNode p_node) const
{
  gbtGameNodeBase *node = dynamic_cast<gbtGameNodeBase *>(p_node.Get());
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

void gbtGameInfosetBase::Reveal(gbtGamePlayer p_who)
{
  m_player->m_efg->Reveal(this,
			  dynamic_cast<gbtGamePlayerBase *>(p_who.Get()));
}

void gbtGameInfosetBase::MergeInfoset(gbtGameInfoset p_from)
{
  gbtGameInfosetBase *from = dynamic_cast<gbtGameInfosetBase *>(p_from.Get());
  if (this == from ||
      m_actions.Length() != from->m_actions.Length())  {
    return;
  }

  m_player->m_efg->MergeInfoset(this, from);
}

gbtGameAction gbtGameInfosetBase::InsertAction(int where)
{
  gbtGameActionBase *action = new gbtGameActionBase(this, where);
  m_actions.Insert(action, where);
  if (m_player->m_id == 0) {
    m_chanceProbs.Insert(0, where);
  }
  for (; where <= m_actions.Length(); where++) {
    m_actions[where]->m_id = where;
  }
  return action;
}

void gbtGameInfosetBase::SetChanceProb(int p_action, const gbtNumber &p_value)
{
  m_chanceProbs[p_action] = p_value;
}

gbtNumber gbtGameInfosetBase::GetChanceProb(int p_action) const
{
  return m_chanceProbs[p_action];
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtGameInfoset &)
{ return p_stream; }
