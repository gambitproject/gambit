//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to supports for extensive forms
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

#ifndef EFGSUPPORT_H
#define EFGSUPPORT_H

#include "base/gstatus.h"
#include "game.h"

//
// A gbtEfgSupport is an extensive form game, restricted to a subset of
// the actions in the game.
//
class gbtEfgSupportRep : public gbtConstEfgRep {
  friend class gbtEfgSupport;
public:
  virtual ~gbtEfgSupportRep() { }

  virtual gbtEfgSupportRep *Copy(void) const = 0;

  virtual bool operator==(const gbtEfgSupportRep &) const = 0;
  bool operator!=(const gbtEfgSupportRep &p_support) const
  { return !(*this == p_support); }

  virtual gbtGame GetTree(void) const = 0;

  virtual void SetLabel(const gbtText &p_label) = 0;

  virtual int NumActions(int pl, int iset) const = 0;
  virtual int NumActions(const gbtGameInfoset &) const = 0;
  virtual gbtPVector<int> NumActions(void) const = 0;
  virtual int NumDegreesOfFreedom(void) const = 0;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  virtual bool HasActiveActionAt(const gbtGameInfoset &) const = 0;
  virtual bool HasActiveActionsAtAllInfosets(void) const = 0;

  virtual bool Contains(const gbtGameAction &) const = 0;
  virtual bool Contains(int pl, int iset, int act) const = 0;
  virtual int GetIndex(const gbtGameAction &) const = 0;
  virtual gbtGameAction GetAction(const gbtGameInfoset &, int index) const = 0;
  virtual gbtGameAction GetAction(int pl, int iset, int index) const = 0;

  // Action editing functions
  virtual void AddAction(const gbtGameAction &) = 0;
  virtual bool RemoveAction(const gbtGameAction &) = 0;

  // Number of Sequences for the player
  virtual int NumSequences(int pl) const = 0;
  virtual int TotalNumSequences(void) const = 0;

  // Reachable Nodes and Information Sets
  virtual gbtList<gbtGameNode> ReachableNonterminalNodes(const gbtGameNode &) const = 0;
  virtual gbtList<gbtGameNode> ReachableNonterminalNodes(const gbtGameNode &,
							 const gbtGameAction &) const = 0;
  virtual gbtList<gbtGameInfoset> ReachableInfosets(const gbtGameNode &) const = 0;
  virtual gbtList<gbtGameInfoset> ReachableInfosets(const gbtGameNode &,
						    const gbtGameAction &) const = 0;
  virtual gbtList<gbtGameInfoset> ReachableInfosets(const gbtGamePlayer &) const = 0;

  virtual bool AlwaysReaches(const gbtGameInfoset &) const = 0;
  virtual bool AlwaysReachesFrom(const gbtGameInfoset &, const gbtGameNode &) const = 0;
  virtual bool MayReach(const gbtGameNode &) const = 0;
  virtual bool MayReach(const gbtGameInfoset &) const = 0;

  virtual bool Dominates(const gbtGameAction &, const gbtGameAction &,
			 bool strong, bool conditional) const = 0;
  virtual bool IsDominated(const gbtGameAction &,
			   bool strong, bool conditional) const = 0;
  virtual gbtEfgSupport Undominated(bool strong, bool conditional,
				    const gbtArray<int> &players,
				    gbtOutput &, // tracefile 
				    gbtStatus &status) const = 0;

  virtual void Dump(gbtOutput &) const = 0;

  // The subsequent members were merged from a derived class.

  // Find the reachable nodes at an infoset
  virtual gbtList<gbtGameNode> ReachableNodesInInfoset(const gbtGameInfoset &) const = 0;

  virtual bool HasActiveActionsAtActiveInfosets(void) = 0;
  virtual bool HasActiveActionsAtActiveInfosetsAndNoOthers(void) = 0;

  virtual bool InfosetIsActive(const gbtGameInfoset &) const = 0;

  virtual bool RemoveActionReturningDeletedInfosets(const gbtGameAction &,
						    gbtList<gbtGameInfoset> *) = 0;
  // Information
  virtual bool InfosetIsActive(const int pl, const int iset) const = 0;
  virtual int  NumActiveNodes(const int pl, const int iset) const = 0;
  virtual int  NumActiveNodes(const gbtGameInfoset &) const = 0;
  virtual bool NodeIsActive(const int pl, const int iset, const int node) const = 0;
  virtual bool NodeIsActive(const gbtGameNode &) const = 0;

  virtual gbtList<gbtGameNode> ReachableNonterminalNodes(void) const = 0;
};

class gbtEfgSupport {
private:
  gbtEfgSupportRep *m_rep;

public:
  gbtEfgSupport(void) : m_rep(0) { }
  gbtEfgSupport(gbtEfgSupportRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgSupport(const gbtEfgSupport &p_support)
    : m_rep(p_support.m_rep->Copy()) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgSupport() { if (m_rep && m_rep->Dereference()) delete m_rep; }
  
  gbtEfgSupport &operator=(const gbtEfgSupport &p_support) {
    if (this != &p_support) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_support.m_rep->Copy();
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  // Equality semantics are defined as having the same support, not
  // the same underlying object.
  bool operator==(const gbtEfgSupport &p_support) const
  { return (*m_rep == *p_support.m_rep); }
  bool operator!=(const gbtEfgSupport &p_support) const
  { return (*m_rep != *p_support.m_rep); }
  
  gbtEfgSupportRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtEfgSupportRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }

  gbtEfgSupportRep *Get(void) const { return m_rep; }
  
  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};


gbtOutput &operator<<(gbtOutput &f, const gbtEfgSupport &);

#endif  // EFGSUPPORT_H
