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

class gbtGameInfoset;
class gbtGameNode;
class gbtGamePlayer;
class gbtGameStrategy;

class gbtGameActionBase;

class gbtGameActionRep : public gbtGameObject {
friend class gbtGameAction;
public:
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  virtual gbtGameInfoset GetInfoset(void) const = 0;

  virtual gbtNumber GetChanceProb(void) const = 0;
  virtual bool Precedes(gbtGameNode) const = 0;

  virtual void DeleteAction(void) = 0;
};

class gbtGameAction {
friend class gbtGame;
private:
  gbtGameActionRep *m_rep;

public:
  gbtGameAction(void) : m_rep(0) { }
  gbtGameAction(gbtGameActionRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtGameAction(const gbtGameAction &p_action)
    : m_rep(p_action.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtGameAction() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtGameAction &operator=(const gbtGameAction &p_action) {
    if (this != &p_action) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_action.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtGameAction &p_action) const
  { return (m_rep == p_action.m_rep); }
  bool operator!=(const gbtGameAction &p_action) const
  { return (m_rep != p_action.m_rep); }

  gbtGameActionRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtGameActionRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  
  gbtGameActionRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtOutput &operator<<(gbtOutput &, const gbtGameAction &);

//-----------------------------------------------------------------------


class gbtGameInfosetRep : public gbtGameObject {
friend class gbtGameInfoset;
public:
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  virtual void DeleteInfoset(void) = 0;

  virtual bool IsChanceInfoset(void) const = 0;

  virtual gbtGamePlayer GetPlayer(void) const = 0;
  virtual void SetPlayer(gbtGamePlayer) = 0;
  
  virtual void SetChanceProb(int act, const gbtNumber &value) = 0; 
  virtual gbtNumber GetChanceProb(int act) const = 0;

  virtual gbtGameAction InsertAction(int where) = 0;

  virtual gbtGameAction GetAction(int act) const = 0;
  virtual int NumActions(void) const = 0;

  virtual gbtGameNode GetMember(int m) const = 0;
  virtual int NumMembers(void) const = 0;

  virtual bool Precedes(gbtGameNode) const = 0;

  virtual void MergeInfoset(gbtGameInfoset from) = 0;
  virtual void Reveal(gbtGamePlayer) = 0;
};


class gbtGameInfoset {
friend class gbtGame;
private:
  gbtGameInfosetRep *m_rep;

public:
  gbtGameInfoset(void) : m_rep(0) { }
  gbtGameInfoset(gbtGameInfosetRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtGameInfoset(const gbtGameInfoset &p_infoset)
    : m_rep(p_infoset.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtGameInfoset() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtGameInfoset &operator=(const gbtGameInfoset &p_infoset) {
    if (this != &p_infoset) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_infoset.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtGameInfoset &p_infoset) const
  { return (m_rep == p_infoset.m_rep); }
  bool operator!=(const gbtGameInfoset &p_infoset) const
  { return (m_rep != p_infoset.m_rep); }

  gbtGameInfosetRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtGameInfosetRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  
  gbtGameInfosetRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtOutput &operator<<(gbtOutput &, const gbtGameInfoset &);

#endif   // INFOSET_H
