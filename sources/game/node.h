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

class gbtGameConstNodeRep : public gbtGameObject {
public:
  virtual int GetId(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;

  virtual int NumChildren(void) const = 0;
  virtual gbtGameNode GetChild(int i) const = 0;
  virtual gbtGameNode GetChild(const gbtGameAction &) const = 0; 
  bool IsTerminal(void) const { return (NumChildren() == 0); }
  bool IsNonterminal(void) const { return !IsTerminal(); }
  virtual bool IsRoot(void) const = 0;
  virtual bool IsPredecessorOf(const gbtGameNode &) const = 0;

  virtual gbtGameNode GetParent(void) const = 0;
  virtual gbtGameAction GetPriorAction(void) const = 0;

  virtual gbtGameNode GetPriorSibling(void) const = 0;
  virtual gbtGameNode GetNextSibling(void) const = 0;

  virtual gbtGameInfoset GetInfoset(void) const = 0;
  virtual int GetMemberId(void) const = 0;
  virtual gbtGameNode GetPriorMember(void) const = 0;
  virtual gbtGameNode GetNextMember(void) const = 0;

  virtual gbtGamePlayer GetPlayer(void) const = 0;

  virtual gbtGameNode GetSubgameRoot(void) const = 0;
  virtual bool IsSubgameRoot(void) const = 0;

  virtual gbtGameOutcome GetOutcome(void) const = 0;
};

class gbtGameNodeRep : public gbtGameConstNodeRep {
friend class gbtGame;
friend class gbtGameInfosetBase;
friend class gbtGameNode;
public:
  virtual void SetLabel(const gbtText &) = 0;

  virtual void SetOutcome(const gbtGameOutcome &) = 0;

  virtual gbtGameNode InsertMove(gbtGameInfoset) = 0;
  // Note: Starting in 0.97.1.1, this now deletes the *parent* move
  virtual void DeleteMove(void) = 0;
  virtual void DeleteTree(void) = 0;

  virtual void JoinInfoset(gbtGameInfoset) = 0;
  virtual gbtGameInfoset LeaveInfoset(void) = 0;
};

class gbtEfgNullNode { };

class gbtGameNode {
friend class gbtGame;
private:
  gbtGameNodeRep *m_rep;

public:
  gbtGameNode(void) : m_rep(0) { }
  gbtGameNode(gbtGameNodeRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtGameNode(const gbtGameNode &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtGameNode() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtGameNode &operator=(const gbtGameNode &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtGameNode &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtGameNode &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtGameNodeRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullNode(); return m_rep; }
  const gbtGameNodeRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullNode(); return m_rep; }
  
  gbtGameNodeRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtOutput &operator<<(gbtOutput &, const gbtGameNode &);

#endif   // NODE_H
