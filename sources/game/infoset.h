//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to objects representing information sets
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

#ifndef INFOSET_H
#define INFOSET_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "math/gvector.h"

#include "efplayer.h"

class Node;
class Lexicon;

struct gbt_efg_action_rep;

class gbtEfgAction {
friend class efgGame;
protected:
  struct gbt_efg_action_rep *rep;

public:
  gbtEfgAction(void);
  gbtEfgAction(gbt_efg_action_rep *);
  gbtEfgAction(const gbtEfgAction &);
  ~gbtEfgAction();

  gbtEfgAction &operator=(const gbtEfgAction &);

  bool operator==(const gbtEfgAction &) const;
  bool operator!=(const gbtEfgAction &) const;

  bool IsNull(void) const;
  int GetId(void) const;
  Infoset *GetInfoset(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);

  bool Precedes(const Node *) const;
};

gOutput &operator<<(gOutput &, const gbtEfgAction &);

#ifdef UNUSED
class Action   {
  friend class efgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class Infoset;
  private:
    int number;
    gText name;
    Infoset *owner;

    Action(int br, const gText &n, Infoset *s)
      : number(br), name(n), owner(s)  { }
    ~Action()   { }

  public:
    const gText &GetLabel(void) const   { return name; }
    void SetLabel(const gText &s)       { name = s; }

    int GetId(void) const        { return number; }
    Infoset *GetInfoset(void) const   { return owner; }
    bool Precedes(const Node *) const;
};
#endif  // UNUSED

class Infoset   {
  friend class efgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class Lexicon;

  protected:
    efgGame *E;
    int number;
    gText name;
    gbt_efg_player_rep *player;
    gBlock<gbt_efg_action_rep *> actions;
    gBlock<Node *> members;
    int flag, whichbranch;
    
    Infoset(efgGame *e, int n, gbt_efg_player_rep *p, int br);
    virtual ~Infoset();  

    virtual void PrintActions(gOutput &f) const;

  public:
    efgGame *Game(void) const   { return E; }

    virtual bool IsChanceInfoset(void) const { return false; }
    gbtEfgPlayer GetPlayer(void) const;

    void SetName(const gText &s)    { name = s; }
    const gText &GetName(void) const   { return name; }

    virtual gbtEfgAction InsertAction(int where);
    virtual void RemoveAction(int which);

    gbtEfgAction GetAction(int act) const;
    int NumActions(void) const   { return actions.Length(); }

    Node *GetMember(int m) const { return members[m]; }
    int NumMembers(void) const   { return members.Length(); }

    int GetNumber(void) const    { return number; }

    bool Precedes(const Node *) const;
};

class ChanceInfoset : public Infoset  {
  friend class efgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class PureBehavProfile<double>;
  friend class PureBehavProfile<gRational>;
  friend class PureBehavProfile<gNumber>;

  private:
    gBlock<gNumber> probs;

    ChanceInfoset(efgGame *E, int n, gbt_efg_player_rep *p, int br);
    virtual ~ChanceInfoset()    { }

    void PrintActions(gOutput &f) const;

  public:
    bool IsChanceInfoset(void) const { return true; }

    gbtEfgAction InsertAction(int where);
    void RemoveAction(int which);

    void SetActionProb(int i, const gNumber &value)  { probs[i] = value; }
    const gNumber &GetActionProb(int i) const   { return probs[i]; }
    const gArray<gNumber> &GetActionProbs(void) const  { return probs; }
};

#endif   //# INFOSET_H
