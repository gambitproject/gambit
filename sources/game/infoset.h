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

#include "math/gvector.h"
#include "game.h"

class gbtEfgInfoset;
class gbtEfgNode;
class gbtEfgPlayer;

class gbtEfgActionBase;

class gbtEfgActionRep : public gbtGameObject {
friend class gbtEfgAction;
public:
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  virtual gbtEfgInfoset GetInfoset(void) const = 0;

  virtual gbtNumber GetChanceProb(void) const = 0;
  virtual bool Precedes(gbtEfgNode) const = 0;

  virtual void DeleteAction(void) = 0;
};

class gbtEfgNullAction { };

class gbtEfgAction {
friend class gbtEfgGame;
private:
  gbtEfgActionRep *m_rep;

public:
  gbtEfgAction(void) : m_rep(0) { }
  gbtEfgAction(gbtEfgActionRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgAction(const gbtEfgAction &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgAction() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtEfgAction &operator=(const gbtEfgAction &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtEfgAction &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtEfgAction &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtEfgActionRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullAction(); return m_rep; }
  const gbtEfgActionRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullAction(); return m_rep; }
  
  gbtEfgActionRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};


gbtOutput &operator<<(gbtOutput &, const gbtEfgAction &);

class gbtEfgInfoset : public gbtGameInfoset {
friend class gbtEfgGame;
friend class gbtEfgNode;
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
  gbtEfgGame GetGame(void) const;
  gbtText GetLabel(void) const;
  void SetLabel(const gbtText &);

  void DeleteInfoset(void);

  bool IsChanceInfoset(void) const;

  gbtEfgPlayer GetPlayer(void) const;
  void SetPlayer(gbtEfgPlayer);
  
  void SetChanceProb(int act, const gbtNumber &value);
  gbtNumber GetChanceProb(int act) const;

  gbtEfgAction InsertAction(int where);

  gbtEfgAction GetAction(int act) const;
  int NumActions(void) const;

  gbtEfgNode GetMember(int m) const;
  int NumMembers(void) const;

  bool Precedes(gbtEfgNode) const;

  void MergeInfoset(gbtEfgInfoset from);
  void Reveal(gbtEfgPlayer);

  bool GetFlag(void) const;
  void SetFlag(bool);

  int GetWhichBranch(void) const;
  void SetWhichBranch(int);
};

gbtOutput &operator<<(gbtOutput &, const gbtEfgInfoset &);

#endif   //# INFOSET_H
