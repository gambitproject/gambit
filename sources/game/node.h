//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to class representing nodes in extensive form games
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

#ifndef NODE_H
#define NODE_H

#include "math/rational.h"
#include "efg.h"

template <class T> class gbtBehavAssessment;

class gbtEfgNodeRep : public gbtGameObject {
friend class gbtEfgGame;
friend class gbtEfgInfosetBase;
friend class gbtEfgNode;
friend struct gbt_efg_game_rep;
public:
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;
  virtual gbtEfgGame GetGame(void) const = 0;

  virtual int NumChildren(void) const = 0;
  bool IsTerminal(void) const { return (NumChildren() == 0); }
  bool IsNonterminal(void) const { return !IsTerminal(); }
  virtual gbtEfgNode GetChild(int i) const = 0;
  virtual gbtEfgNode GetChild(const gbtEfgAction &) const = 0; 
  virtual bool IsPredecessorOf(const gbtEfgNode &) const = 0;

  virtual gbtEfgNode GetParent(void) const = 0;
  virtual gbtEfgAction GetPriorAction(void) const = 0; // returns null if root node

  virtual gbtEfgNode GetPriorSibling(void) const = 0;
  virtual gbtEfgNode GetNextSibling(void) const = 0;

  virtual gbtEfgInfoset GetInfoset(void) const = 0;
  virtual int GetMemberId(void) const = 0;
  virtual gbtEfgNode GetPriorMember(void) const = 0;
  virtual gbtEfgNode GetNextMember(void) const = 0;

  virtual gbtEfgPlayer GetPlayer(void) const = 0;

  virtual gbtEfgNode GetSubgameRoot(void) const = 0;
  virtual bool IsSubgameRoot(void) const = 0;

  virtual gbtEfgOutcome GetOutcome(void) const = 0;
  virtual void SetOutcome(const gbtEfgOutcome &) = 0;

  virtual gbtEfgNode InsertMove(gbtEfgInfoset) = 0;
  // Note: Starting in 0.97.1.1, this now deletes the *parent* move
  virtual void DeleteMove(void) = 0;
  virtual void DeleteTree(void) = 0;

  virtual void JoinInfoset(gbtEfgInfoset) = 0;
  virtual gbtEfgInfoset LeaveInfoset(void) = 0;
};

class gbtEfgNullNode { };

class gbtEfgNode {
friend class gbtEfgGame;
private:
  gbtEfgNodeRep *m_rep;

public:
  gbtEfgNode(void) : m_rep(0) { }
  gbtEfgNode(gbtEfgNodeRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgNode(const gbtEfgNode &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgNode() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtEfgNode &operator=(const gbtEfgNode &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtEfgNode &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtEfgNode &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtEfgNodeRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullAction(); return m_rep; }
  const gbtEfgNodeRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullAction(); return m_rep; }
  
  gbtEfgNodeRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtOutput &operator<<(gbtOutput &, const gbtEfgNode &);

#endif   // NODE_H
