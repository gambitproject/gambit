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

#ifndef GAME_NODE_H
#define GAME_NODE_H

#include "game.h"

class gbtGameNodeRep;
typedef gbtGameObjectHandle<gbtGameNodeRep> gbtGameNode;

class gbtGameNodeRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameNodeRep>;
  friend struct gbt_efg_game_rep;

public:
  //!
  //! @name General information about the node
  //!
  //@{
  /// Returns the ID number of the node (unique within a tree)
  virtual int GetId(void) const = 0;

  /// Set the text label associated with the node.
  virtual void SetLabel(const std::string &) = 0;
  /// Get the text label associated with the node.
  virtual std::string GetLabel(void) const = 0;
  //@}

  //!
  //! @name Accessing relatives of the node
  //!
  //@{
  /// Returns the number of children (direct descendants) of the node
  virtual int NumChildren(void) const = 0;
  /// Returns true if the node is terminal (no children)
  virtual bool IsTerminal(void) const = 0;
  /// Returns true if the node is nonterminal (has children)
  virtual bool IsNonterminal(void) const = 0;
  /// Returns the ith child of the node.
  virtual gbtGameNode GetChild(int i) const = 0;
  /// Returns the child of the node obtained by following the action.
  virtual gbtGameNode GetChild(const gbtGameAction &) const = 0; 
  /// Returns true if this node precedes the node in the tree.
  virtual bool IsPredecessorOf(const gbtGameNode &) const = 0;

  /// Returns the parent of the node; the root node has a null parent.
  virtual gbtGameNode GetParent(void) const = 0;
  /// Returns the action directly before the node
  virtual gbtGameAction GetPriorAction(void) const = 0; 
  /// Returns the sequence for the player that leads to the node
  virtual gbtGameSequence GetSequence(const gbtGamePlayer &) const = 0;

  /// Returns the prior sibling of the node
  virtual gbtGameNode GetPriorSibling(void) const = 0;
  /// Returns the subsequent sibling of the node
  virtual gbtGameNode GetNextSibling(void) const = 0;

  //@}

  //!
  //! @name Accessing the information set of the node
  //!
  //@{
  /// Returns the information set to which the node belongs.
  virtual gbtGameInfoset GetInfoset(void) const = 0;
  /// Returns the ID number of the node within the information set.
  virtual int GetMemberId(void) const = 0;
  /// Returns the prior member of the information set.
  virtual gbtGameNode GetPriorMember(void) const = 0;
  /// Returns the next member of the information set.
  virtual gbtGameNode GetNextMember(void) const = 0;
  /// Returns the player who has the move at the node.
  virtual gbtGamePlayer GetPlayer(void) const = 0;
  //@}

  //!
  //! @name Accessing the outcome at a node
  //!
  //@{
  /// Returns the outcome at the node (may be null).
  virtual gbtGameOutcome GetOutcome(void) const = 0;
  /// Sets the outcome at the node (may be null).
  virtual void SetOutcome(const gbtGameOutcome &) = 0;
  //@}

  //!
  //! @name Editing the game tree
  //!
  //@{
  /// Insert a new move before the node, belonging to the information set.
  virtual gbtGameNode InsertMove(gbtGameInfoset) = 0;
  /// Delete the parent of this node, and take its place in the tree.
  virtual void DeleteMove(void) = 0;
  /// Delete the subtree rooted at this node.
  virtual void DeleteTree(void) = 0;
  /// Copy the subtree rooted at src to this node.
  virtual gbtGameNode CopyTree(gbtGameNode src) = 0;
  /// Move the subtree rooted at src to this node.
  virtual gbtGameNode MoveTree(gbtGameNode src) = 0;

  /// Place this node in the specified information set.
  virtual void JoinInfoset(const gbtGameInfoset &) = 0;
  /// Leave the current information set and become a new singleton infoset.
  virtual gbtGameInfoset LeaveInfoset(void) = 0;
  //@}
};

typedef gbtGameObjectHandle<gbtGameNodeRep> gbtGameNode;


#endif   // GAME_NODE_H
