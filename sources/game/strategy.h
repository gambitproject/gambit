//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to represent a normal form (pure) strategy
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

#ifndef STRATEGY_H
#define STRATEGY_H

#include "game.h"

class gbtNfgActionRep;
class gbtNfgPlayer;

class gbtNfgActionRep : public gbtGameObject {
friend class gbtNfgAction;
public:
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  //  virtual gbtNfgInfoset GetInfoset(void) const = 0;

  virtual gbtNfgPlayer GetPlayer(void) const = 0;
  virtual long GetIndex(void) const = 0;

  virtual const gbtArray<int> *const GetBehavior(void) const = 0;
};

class gbtNfgNullAction { };

class gbtNfgAction {
friend class gbtNfgGame;
private:
  gbtNfgActionRep *m_rep;

public:
  gbtNfgAction(void) : m_rep(0) { }
  gbtNfgAction(gbtNfgActionRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgAction(const gbtNfgAction &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgAction() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtNfgAction &operator=(const gbtNfgAction &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtNfgAction &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtNfgAction &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtNfgActionRep *operator->(void) 
  { if (!m_rep) throw gbtNfgNullAction(); return m_rep; }
  const gbtNfgActionRep *operator->(void) const 
  { if (!m_rep) throw gbtNfgNullAction(); return m_rep; }
  
  gbtNfgActionRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtOutput &operator<<(gbtOutput &, const gbtNfgAction &);

#endif  // STRATEGY_H
