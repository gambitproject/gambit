//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of nodes for explicit game trees
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

#ifndef TREE_NODE_H
#define TREE_NODE_H

class gbtTreeNodeRep : public gbtGameNodeRep {
public:
  int m_refCount, m_id;
  gbtTreeGameRep *m_efg;
  bool m_deleted;
  std::string m_label;

  gbtTreeInfosetRep *m_infoset;
  gbtTreeNodeRep *m_parent;
  gbtTreeOutcomeRep *m_outcome;
  gbtBlock<gbtTreeNodeRep *> m_children;
  gbtTreeNodeRep *m_whichbranch;
  mutable gbtTreeNodeRep *m_ptr;

  /// @name Constructor and destructor
  //@{
  gbtTreeNodeRep(gbtTreeGameRep *, gbtTreeNodeRep *);
  virtual ~gbtTreeNodeRep();
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  void Delete(void)
    { if (m_refCount == 0) delete this; else m_deleted = true; }
  //@}

  /// @name General information about the node
  //@{
  int GetId(void) const;
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  bool IsDeleted(void) const { return m_deleted; }
  //@}

  /// @name Accessing relatives of the node
  //@{
  int NumChildren(void) const;
  bool IsTerminal(void) const;
  bool IsNonterminal(void) const;
  gbtGameNode GetChild(int) const;
  gbtGameNode GetChild(const gbtGameAction &) const;
  bool IsPredecessorOf(const gbtGameNode &) const;
  gbtGameNode GetParent(void) const;
  gbtGameAction GetPriorAction(void) const;
  gbtGameNode GetPriorSibling(void) const;
  gbtGameNode GetNextSibling(void) const;
  //@}

  /// @name Accessing the information set of the node
  //@{
  gbtGameInfoset GetInfoset(void) const;
  int GetMemberId(void) const;
  gbtGameNode GetPriorMember(void) const;
  gbtGameNode GetNextMember(void) const;
  gbtGamePlayer GetPlayer(void) const;
  //@}

  /// @name Accessing the outcome at a node
  //@{
  void SetOutcome(const gbtGameOutcome &);
  gbtGameOutcome GetOutcome(void) const;
  //@}

  /// @name Editing the game tree
  //@{
  gbtGameNode InsertMove(gbtGameInfoset);
  void DeleteMove(void);
  void DeleteTree(void);
  gbtGameNode CopyTree(gbtGameNode);
  gbtGameNode MoveTree(gbtGameNode);

  void JoinInfoset(const gbtGameInfoset &);
  gbtGameInfoset LeaveInfoset(void);
  //@}

  /// @name Member functions private to the implementation
  //@{
  void DeleteOutcome(gbtTreeOutcomeRep *outc);
  void DeleteSubtree(void);
  void CopySubtree(gbtTreeNodeRep *, gbtTreeNodeRep *);
  //@}
};

#endif  // TREE_NODE_H
