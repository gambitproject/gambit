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
//           struct gbt_efg_action_rep: Member functions
//----------------------------------------------------------------------

gbt_efg_action_rep::gbt_efg_action_rep(gbt_efg_infoset_rep *p_infoset,
				       int p_id)
  : m_id(p_id), m_infoset(p_infoset), m_deleted(false), 
    m_refCount(0)
{ }

//----------------------------------------------------------------------
//              class gbtEfgAction: Member functions
//----------------------------------------------------------------------

gbtEfgAction::gbtEfgAction(void)
  : rep(0)
{ }

gbtEfgAction::gbtEfgAction(gbt_efg_action_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_infoset->m_player->m_efg->m_refCount++;
  }
}

gbtEfgAction::gbtEfgAction(const gbtEfgAction &p_action)
  : rep(p_action.rep)
{
  if (rep) {
    rep->m_refCount++;
    if (!rep->m_deleted) {
      rep->m_infoset->m_player->m_efg->m_refCount++;
    }
  }
}

gbtEfgAction::~gbtEfgAction()
{
  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      delete rep;
    }
    else if (--rep->m_infoset->m_player->m_efg->m_refCount == 0) {
      delete rep->m_infoset->m_player->m_efg;
    }
  }
}

gbtEfgAction &gbtEfgAction::operator=(const gbtEfgAction &p_action)
{
  if (this == &p_action) {
    return *this;
  }

  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      delete rep;
    }
    else if (--rep->m_infoset->m_player->m_efg->m_refCount == 0) {
      delete rep->m_infoset->m_player->m_efg;
    }
  }

  if ((rep = p_action.rep) != 0) {
    rep->m_refCount++;
    if (!rep->m_deleted) {
      rep->m_infoset->m_player->m_efg->m_refCount++;
    }
  }
  return *this;
}

bool gbtEfgAction::operator==(const gbtEfgAction &p_action) const
{
  return (rep == p_action.rep);
} 

bool gbtEfgAction::operator!=(const gbtEfgAction &p_action) const
{
  return (rep != p_action.rep);
} 

bool gbtEfgAction::IsNull(void) const
{
  return (rep == 0);
}

int gbtEfgAction::GetId(void) const
{
  return (rep) ? rep->m_id : -1;
}

gbtText gbtEfgAction::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtEfgAction::SetLabel(const gbtText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

gbtEfgInfoset gbtEfgAction::GetInfoset(void) const
{
  if (rep) {
    return rep->m_infoset;
  }
  else {
    return 0;
  }
}

gbtNumber gbtEfgAction::GetChanceProb(void) const
{
  if (rep) {
    return rep->m_infoset->m_chanceProbs[rep->m_id];
  }
  else {
    return 0;
  }
}

bool gbtEfgAction::Precedes(gbtEfgNode n) const
{
  if (!rep) {
    return false;
  }

  while (n != n.GetGame().GetRoot() ) {
    if (n.GetPriorAction().rep == rep) {
      return true;
    }
    else {
      n = n.GetParent();
    }
  }
  return false;
}

void gbtEfgAction::DeleteAction(void)
{
  if (IsNull()) {
    throw gbtEfgNullObject();
  }

  if (rep->m_infoset->m_actions.Length() == 1) {
    return;
  }

  rep->m_infoset->m_player->m_efg->DeleteAction(rep->m_infoset, rep);
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgAction &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//           struct gbt_efg_infoset_rep: Member functions
//----------------------------------------------------------------------

gbt_efg_infoset_rep::gbt_efg_infoset_rep(gbt_efg_player_rep *p_player,
					 int p_id, int p_br)
  : m_id(p_id), m_player(p_player), m_deleted(false), 
    m_refCount(0), m_actions(p_br),
    m_chanceProbs((p_player->m_id == 0) ? p_br : 0),
    m_flag(0), m_whichbranch(0)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbt_efg_action_rep(this, act);
    if (p_player->m_id == 0) {
      m_chanceProbs[act] = gbtRational(1, p_br);
    }
  }
}

gbt_efg_infoset_rep::~gbt_efg_infoset_rep()
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

void gbt_efg_infoset_rep::PrintActions(gbtOutput &f) const
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


//----------------------------------------------------------------------
//              class gbtEfgInfoset: Member functions
//----------------------------------------------------------------------

gbtEfgInfoset::gbtEfgInfoset(void)
  : rep(0)
{ }

gbtEfgInfoset::gbtEfgInfoset(gbt_efg_infoset_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_player->m_efg->m_refCount++;
  }
}

gbtEfgInfoset::gbtEfgInfoset(const gbtEfgInfoset &p_action)
  : rep(p_action.rep)
{
  if (rep) {
    rep->m_refCount++;
    rep->m_player->m_efg->m_refCount++;
  }
}

gbtEfgInfoset::~gbtEfgInfoset()
{
  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      // delete rep;
    }
    else if (--rep->m_player->m_efg->m_refCount == 0) {
      // delete rep->m_player->m_efg;
    }
  }
}

gbtEfgInfoset &gbtEfgInfoset::operator=(const gbtEfgInfoset &p_infoset)
{
  if (this == &p_infoset) {
    return *this;
  }

  if (rep) {
    if (--rep->m_refCount == 0 && rep->m_deleted) {
      // delete rep;
    }
    else if (--rep->m_player->m_efg->m_refCount == 0) {
      // delete rep->m_player->m_efg;
    }
  }

  if ((rep = p_infoset.rep) != 0) {
    rep->m_refCount++;
    rep->m_player->m_efg->m_refCount++;
  }
  return *this;
}

bool gbtEfgInfoset::operator==(const gbtEfgInfoset &p_infoset) const
{
  return (rep == p_infoset.rep);
} 

bool gbtEfgInfoset::operator!=(const gbtEfgInfoset &p_infoset) const
{
  return (rep != p_infoset.rep);
} 

bool gbtEfgInfoset::IsNull(void) const
{
  return (rep == 0);
}

int gbtEfgInfoset::GetId(void) const
{
  return (rep) ? rep->m_id : -1;
}

gbtText gbtEfgInfoset::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtEfgInfoset::SetLabel(const gbtText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

void gbtEfgInfoset::DeleteInfoset(void)
{
  if (IsNull())  {
    throw gbtEfgNullObject();
  }

  if (NumMembers() > 0)  {
    return;
  }

  rep->m_player->m_efg->DeleteInfoset(rep);
}

gbtEfgAction gbtEfgInfoset::GetAction(int p_index) const
{
  if (rep) {
    return rep->m_actions[p_index];
  }
  else {
    return 0;
  }
}

int gbtEfgInfoset::NumActions(void) const
{
  if (rep) {
    return rep->m_actions.Length();
  }
  else {
    return 0;
  }
}

gbtEfgNode gbtEfgInfoset::GetMember(int p_index) const
{
  if (rep) {
    return rep->m_members[p_index];
  }
  else {
    return 0;
  }
}

int gbtEfgInfoset::NumMembers(void) const
{
  if (rep) {
    return rep->m_members.Length();
  }
  else {
    return 0;
  }
}

gbtEfgPlayer gbtEfgInfoset::GetPlayer(void) const
{
  if (rep) {
    return rep->m_player;
  }
  else {
    return 0;
  }
}

void gbtEfgInfoset::SetPlayer(gbtEfgPlayer p_player)
{
  if (IsNull() || p_player.IsNull()) {
    throw gbtEfgNullObject();
  }
  if (GetPlayer().IsChance() || p_player.IsChance()) {
    throw gbtEfgbtException();
  }
  
  if (GetPlayer() == p_player) {
    return;
  }

  rep->m_player->m_efg->SetPlayer(rep, p_player.rep);
}

bool gbtEfgInfoset::IsChanceInfoset(void) const
{
  return (rep && rep->m_player->m_id == 0);
}

gbtEfgGame gbtEfgInfoset::GetGame(void) const
{
  if (rep) {
    return rep->m_player->m_efg;
  }
  else {
    return 0;
  }
}

bool gbtEfgInfoset::Precedes(gbtEfgNode p_node) const
{
  while (p_node != p_node.GetGame().GetRoot()) {
    if (p_node.GetInfoset() == *this) {
      return true;
    }
    else {
      p_node = p_node.GetParent();
    }
  }
  return false;
}

void gbtEfgInfoset::Reveal(gbtEfgPlayer p_who)
{
  if (IsNull() || p_who.IsNull()) {
    return;
  }

  rep->m_player->m_efg->Reveal(rep, p_who.rep);
}

void gbtEfgInfoset::MergeInfoset(gbtEfgInfoset p_from)
{
  if (IsNull() || p_from.IsNull()) {
    throw gbtEfgNullObject();
  }

  if (rep == p_from.rep ||
      rep->m_actions.Length() != p_from.rep->m_actions.Length())  {
    return;
  }

  // FIXME: Can't bridge subgames
  if (rep->m_members[1]->m_gameroot != p_from.rep->m_members[1]->m_gameroot) {
    return;
  }

  rep->m_player->m_efg->MergeInfoset(rep, p_from.rep);
}

gbtEfgAction gbtEfgInfoset::InsertAction(int where)
{
  gbt_efg_action_rep *action = new gbt_efg_action_rep(rep, where);
  rep->m_actions.Insert(action, where);
  if (rep->m_player->m_id == 0) {
    rep->m_chanceProbs.Insert(0, where);
  }
  for (; where <= rep->m_actions.Length(); where++) {
    rep->m_actions[where]->m_id = where;
  }
  return action;
}

void gbtEfgInfoset::SetChanceProb(int p_action, const gbtNumber &p_value)
{
  rep->m_chanceProbs[p_action] = p_value;
}

gbtNumber gbtEfgInfoset::GetChanceProb(int p_action) const
{
  if (rep) {
    return rep->m_chanceProbs[p_action];
  }
  else {
    return 0;
  }
}

bool gbtEfgInfoset::GetFlag(void) const
{ 
  return rep->m_flag;
}

void gbtEfgInfoset::SetFlag(bool p_flag)
{
  rep->m_flag = p_flag;
}

int gbtEfgInfoset::GetWhichBranch(void) const
{
  return rep->m_whichbranch;
}

void gbtEfgInfoset::SetWhichBranch(int p_branch) 
{
  rep->m_whichbranch = p_branch;
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgInfoset &)
{ 
  return p_stream;
}
