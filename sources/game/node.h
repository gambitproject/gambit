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

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class Lexicon;

template <class T> class BehavAssessment;

class gbtEfgNode {
friend class gbtEfgGame;
friend struct gbt_efg_game_rep;
friend class Lexicon;
protected:
  struct gbt_efg_node_rep *rep;

public:
  gbtEfgNode(void);
  gbtEfgNode(gbt_efg_node_rep *);
  gbtEfgNode(const gbtEfgNode &);
  ~gbtEfgNode();

  gbtEfgNode &operator=(const gbtEfgNode &);

  bool operator==(const gbtEfgNode &) const;
  bool operator!=(const gbtEfgNode &) const;

  bool IsNull(void) const;
  int GetId(void) const;
  gbtEfgGame GetGame(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);

  int NumChildren(void) const;
  bool IsTerminal(void) const { return (NumChildren() == 0); }
  bool IsNonterminal(void) const { return !IsTerminal(); }
  gbtEfgNode GetChild(int i) const;
  gbtEfgNode GetChild(const gbtEfgAction &) const; 
  bool IsPredecessorOf(const gbtEfgNode &) const;

  gbtEfgNode GetParent(void) const;
  gbtEfgAction GetPriorAction(void) const; // returns null if root node

  gbtEfgNode GetPriorSibling(void) const;
  gbtEfgNode GetNextSibling(void) const;

  gbtEfgInfoset GetInfoset(void) const;
  int GetMemberId(void) const;
  gbtEfgPlayer GetPlayer(void) const;

  gbtEfgNode GetSubgameRoot(void) const;
  bool IsSubgameRoot(void) const;

  gbtEfgOutcome GetOutcome(void) const;
  void SetOutcome(const gbtEfgOutcome &);

  gbtEfgNode InsertMove(gbtEfgInfoset);
  void DeleteMove(gbtEfgNode p_keep);
  void DeleteTree(void);

  void JoinInfoset(gbtEfgInfoset);
  gbtEfgInfoset LeaveInfoset(void);
};

gOutput &operator<<(gOutput &, const gbtEfgNode &);

#endif   // NODE_H






