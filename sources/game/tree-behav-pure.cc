//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Definition of pure behavior profile for trees
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

#include "tree-behav-pure.h"

//======================================================================
//        Implementation of class gbtTreeBehavContingencyRep
//======================================================================

//----------------------------------------------------------------------
//    class gbtTreeBehavContingencyRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeBehavContingencyRep::gbtTreeBehavContingencyRep(gbtTreeGameRep *p_efg)
  : m_refCount(0), m_efg(p_efg), m_profile(m_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    gbtTreePlayerRep *player = m_efg->m_players[pl];
    m_profile[pl] = gbtArray<gbtTreeActionRep *>(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_profile[pl][iset] = player->m_infosets[iset]->m_actions[1];
    }
  }
}

gbtTreeBehavContingencyRep::~gbtTreeBehavContingencyRep()
{ }

gbtGameBehavContingencyRep *gbtTreeBehavContingencyRep::Copy(void) const
{
  gbtTreeBehavContingencyRep *rep = new gbtTreeBehavContingencyRep(m_efg);

  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    gbtTreePlayerRep *player = m_efg->m_players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      rep->m_profile[pl][iset] = m_profile[pl][iset];
    }
  } 

  return rep;
}

//----------------------------------------------------------------------
// class gbtTreeBehavContingencyRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeBehavContingencyRep::Reference(void)
{
  m_refCount++;
  m_efg->m_refCount++;
}

bool gbtTreeBehavContingencyRep::Dereference(void)
{
  if (--m_efg->m_refCount == 0) {
    // Note that if this condition is true, this profile must be the
    // last reference to the game, so m_refCount will also be one
    // (and this function will return true)
    delete m_efg;
  }
  return (--m_refCount == 0);
}


//----------------------------------------------------------------------
//      class gbtTreeBehavContingencyRep: Accessing the state
//----------------------------------------------------------------------

gbtGameAction 
gbtTreeBehavContingencyRep::GetAction(const gbtGameInfoset &p_infoset) const
{
  if (p_infoset.IsNull())  throw gbtGameNullException();
  gbtTreeInfosetRep *infoset =
    dynamic_cast<gbtTreeInfosetRep *>(p_infoset.Get());
  if (!infoset || infoset->m_player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }

  return m_profile[infoset->m_player->m_id][infoset->m_id];
}

void gbtTreeBehavContingencyRep::SetAction(const gbtGameAction &p_action)
{
  if (p_action.IsNull())  throw gbtGameNullException();
  gbtTreeActionRep *action = dynamic_cast<gbtTreeActionRep *>(p_action.Get());
  if (!action || action->m_infoset->m_player->m_efg != m_efg) {
    throw gbtGameMismatchException();
  }
  m_profile[action->m_infoset->m_player->m_id]
    [action->m_infoset->m_id] = action;
}

gbtRational 
gbtTreeBehavContingencyRep::GetPayoff(gbtTreeNodeRep *p_node,
				      gbtTreePlayerRep *p_player) const
{
  gbtRational payoff(0);
  if (p_node->m_outcome) {
    payoff += p_node->m_outcome->m_payoffs[p_player->m_id];
  }

  if (p_node->m_infoset && p_node->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= p_node->m_children.Length(); i++) {
      payoff += (p_node->m_infoset->m_chanceProbs[i] *
		 GetPayoff(p_node->m_children[i], p_player));
    }
  }
  else if (p_node->m_children.Length() > 0) {
    int pl = p_node->m_infoset->m_player->m_id;
    int iset = p_node->m_infoset->m_id;
    payoff += GetPayoff(p_node->m_children[m_profile[pl][iset]->m_id], 
			p_player);
  }

  return payoff;
}

gbtRational 
gbtTreeBehavContingencyRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  if (p_player.IsNull())  throw gbtGameNullException();
  gbtTreePlayerRep *player = dynamic_cast<gbtTreePlayerRep *>(p_player.Get());
  if (!player || player->m_efg != m_efg) throw gbtGameMismatchException();
  return GetPayoff(m_efg->m_root, player);
}


//======================================================================
//      Implementation of class gbtTreeBehavProfileIteratorRep
//======================================================================

//----------------------------------------------------------------------
//  class gbtTreeBehavProfileIteratorRep: Constructor and destructor
//----------------------------------------------------------------------


gbtTreeBehavProfileIteratorRep::gbtTreeBehavProfileIteratorRep(gbtTreeGameRep *p_efg)
  : m_refCount(0), m_efg(p_efg), m_profile(p_efg)
{
  First();
}

gbtGameBehavProfileIteratorRep *
gbtTreeBehavProfileIteratorRep::Copy(void) const
{
  gbtTreeBehavProfileIteratorRep *ret = 
    new gbtTreeBehavProfileIteratorRep(m_efg);

  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= m_efg->GetPlayer(pl)->NumInfosets(); iset++) {
      ret->m_profile.m_profile[pl][iset] = m_profile.m_profile[pl][iset];
    }
  }

  return ret;
}

//----------------------------------------------------------------------
// class gbtTreeBehavProfileIteratorRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeBehavProfileIteratorRep::Reference(void)
{
  m_refCount++;
  m_efg->m_refCount++;
}

bool gbtTreeBehavProfileIteratorRep::Dereference(void)
{
  if (--m_efg->m_refCount == 0) {
    // Note that if this condition is true, this profile must be the
    // last reference to the game, so m_refCount will also be one
    // (and this function will return true)
    delete m_efg;
  }
  return (--m_refCount == 0);
}


//----------------------------------------------------------------------
//          class gbtTreeBehavProfileIteratorRep: Iteration
//----------------------------------------------------------------------

void gbtTreeBehavProfileIteratorRep::First(void)
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= m_efg->GetPlayer(pl)->NumInfosets(); iset++) {
      m_profile.m_profile[pl][iset] = m_efg->m_players[pl]->m_infosets[iset]->m_actions[1];
    }
  }
}

bool gbtTreeBehavProfileIteratorRep::NextContingency(void)
{
  int pl = m_efg->NumPlayers();
  int iset = m_efg->GetPlayer(pl)->NumInfosets();
    
  while (true) {
    if (m_profile.m_profile[pl][iset]->m_id < 
	m_efg->GetPlayer(pl)->GetInfoset(iset)->NumActions())  {
      m_profile.m_profile[pl][iset] = 
	m_efg->m_players[pl]->m_infosets[iset]->m_actions[m_profile.m_profile[pl][iset]->m_id+1];
      return true;
    }
    else {
      m_profile.m_profile[pl][iset] = 
	m_efg->m_players[pl]->m_infosets[iset]->m_actions[1];
    }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0);
      
      if (pl == 0)   return 0;
      iset = m_efg->GetPlayer(pl)->NumInfosets();
    }
  }
}

//----------------------------------------------------------------------
//     class gbtTreeBehavProfileIteratorRep: Accessing the state
//----------------------------------------------------------------------

gbtGameAction 
gbtTreeBehavProfileIteratorRep::GetAction(const gbtGameInfoset &p_infoset) const
{
  return m_profile.GetAction(p_infoset);
}

gbtRational 
gbtTreeBehavProfileIteratorRep::GetPayoff(const gbtGamePlayer &p_player) const
{
  return m_profile.GetPayoff(p_player);
}
