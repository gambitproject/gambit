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

class Lexicon;
class gbtEfgInfoset;
class gbtEfgNode;

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
  gbtEfgInfoset GetInfoset(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);

  gNumber GetChanceProb(void) const;
  bool Precedes(gbtEfgNode) const;
};

gOutput &operator<<(gOutput &, const gbtEfgAction &);

class gbtEfgInfoset {
friend class efgGame;
protected:
  struct gbt_efg_infoset_rep *rep;

public:
  gbtEfgInfoset(void);
  gbtEfgInfoset(gbt_efg_infoset_rep *);
  gbtEfgInfoset(const gbtEfgInfoset &);
  ~gbtEfgInfoset();

  gbtEfgInfoset &operator=(const gbtEfgInfoset &);

  bool operator==(const gbtEfgInfoset &) const;
  bool operator!=(const gbtEfgInfoset &) const;

  bool IsNull(void) const;
  int GetId(void) const;
  efgGame *GetGame(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);

  bool IsChanceInfoset(void) const;
  gbtEfgPlayer GetPlayer(void) const;

  void SetChanceProb(int act, const gNumber &value);
  gNumber GetChanceProb(int act) const;

  gbtEfgAction InsertAction(int where);
  void RemoveAction(int which);

  gbtEfgAction GetAction(int act) const;
  int NumActions(void) const;

  gbtEfgNode GetMember(int m) const;
  int NumMembers(void) const;

  bool Precedes(gbtEfgNode) const;

  bool GetFlag(void) const;
  void SetFlag(bool);

  int GetWhichBranch(void) const;
  void SetWhichBranch(int);
};

gOutput &operator<<(gOutput &, const gbtEfgInfoset &);

#endif   //# INFOSET_H
