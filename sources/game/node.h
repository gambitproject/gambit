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
friend class efgGame;
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
  efgGame *GetGame(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);

  int NumChildren(void) const;
  int NumberInInfoset(void) const;
  gbtEfgInfoset GetInfoset(void) const;
  bool IsTerminal(void) const { return (NumChildren() == 0); }
  bool IsNonterminal(void) const { return !IsTerminal(); }
  gbtEfgPlayer GetPlayer(void) const;
  gbtEfgAction GetAction(void) const; // returns null if root node
  gbtEfgNode GetChild(int i) const;
  gbtEfgNode GetChild(const gbtEfgAction &) const; 
  gbtEfgNode GetParent(void) const;
  gbtEfgNode GetSubgameRoot(void) const;
  gbtEfgNode NextSibling(void) const;
  gbtEfgNode PriorSibling(void) const;

  gbtEfgOutcome GetOutcome(void) const;
  void SetOutcome(const gbtEfgOutcome &);
};

gOutput &operator<<(gOutput &, const gbtEfgNode &);

#ifdef UNUSED
class Node    {
  friend class efgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class BehavAssessment<double>;
  friend class BehavAssessment<gRational>;
  friend class BehavAssessment<gNumber>;
  friend class Lexicon;
  
  protected:
    bool mark;
    int number; // This is a unique identifier, not related to infoset memship
    efgGame *E;
    gText name;
    gbt_efg_infoset_rep *infoset;
    Node *parent;
    gbt_efg_outcome_rep *outcome;
    gBlock<Node *> children;
    Node *whichbranch, *ptr, *gameroot;

    Node(efgGame *e, Node *p);
    ~Node();

    void DeleteOutcome(gbt_efg_outcome_rep *outc);

  public:
    efgGame *GetGame(void) const   { return E; }

    int NumChildren(void) const    { return children.Length(); }
    int GetId(void) const { return number; }
    int NumberInInfoset(void) const;
    gbtEfgNode GetInfoset(void) const;
    bool IsTerminal(void) const { return (children.Length() == 0); }
    bool IsNonterminal(void) const { return !IsTerminal(); }
    gbtEfgPlayer GetPlayer(void) const;
    gbtEfgAction GetAction(void) const; // returns null if root node
    Node *GetChild(int i) const    { return children[i]; }
    Node *GetChild(const gbtEfgAction &) const; 
    Node *GetParent(void) const    { return parent; }
    Node *GetSubgameRoot(void) const  { return gameroot; }
    Node *NextSibling(void) const;
    Node *PriorSibling(void) const;

    const gText &GetLabel(void) const   { return name; }
    void SetLabel(const gText &s)       { name = s; }

    gbtEfgOutcome GetOutcome(void) const;
    void SetOutcome(const gbtEfgOutcome &);
};
#endif  // UNUSED

#endif   // NODE_H






