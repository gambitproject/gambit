//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to player representation classes
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

#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

//
// The extensive form and normal form player classes are both included
// in this header file in the hopes of an eventual unification of the
// two.
//



class gbtEfgPlayerBase;
class gbtEfgGame;
class gbtEfgInfoset;

class gbtNfgPlayer;
class gbtEfgPlayer;
class gbtEfgAction;

class gbtEfgStrategyRep : public gbtGameObject {
friend class gbtEfgStrategy;
public:
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  virtual gbtEfgPlayer GetPlayer(void) const = 0;
  virtual gbtEfgAction GetAction(const gbtEfgInfoset &) const = 0;
  virtual const gbtArray<int> &GetBehavior(void) const = 0;
};

class gbtEfgNullStrategy { };

class gbtEfgStrategy {
friend class gbtNfgGame;
private:
  gbtEfgStrategyRep *m_rep;

public:
  gbtEfgStrategy(void) : m_rep(0) { }
  gbtEfgStrategy(gbtEfgStrategyRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgStrategy(const gbtEfgStrategy &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgStrategy() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtEfgStrategy &operator=(const gbtEfgStrategy &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtEfgStrategy &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtEfgStrategy &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtEfgStrategyRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullStrategy(); return m_rep; }
  const gbtEfgStrategyRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullStrategy(); return m_rep; }
  
  gbtEfgStrategyRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

class gbtEfgPlayerRep : public gbtGameObject {
friend class gbtEfgPlayer;
friend class gbtEfgGame;
friend class gbtEfgInfoset;
public:
  virtual gbtEfgGame GetGame(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  virtual bool IsChance(void) const = 0;

  virtual int NumInfosets(void) const = 0;
  virtual gbtEfgInfoset NewInfoset(int p_actions) = 0;
  virtual gbtEfgInfoset GetInfoset(int p_index) const = 0;
};

class gbtEfgNullPlayer { };

class gbtEfgPlayer {
private:
  gbtEfgPlayerRep *m_rep;

public:
  gbtEfgPlayer(void) : m_rep(0) { }
  gbtEfgPlayer(gbtEfgPlayerRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgPlayer(const gbtEfgPlayer &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgPlayer() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtEfgPlayer &operator=(const gbtEfgPlayer &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtEfgPlayer &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtEfgPlayer &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtEfgPlayerRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullPlayer(); return m_rep; }
  const gbtEfgPlayerRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullPlayer(); return m_rep; }
  
  gbtEfgPlayerRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

//------------------------------------------------------------------------

struct gbtNfgPlayerBase;
class gbtNfgGame;
class gbtNfgInfoset;
class gbtNfgAction;

class gbtNfgPlayerRep : public gbtGameObject {
friend class gbtNfgPlayer;
friend class gbtNfgGame;
friend class gbtNfgInfoset;
public:
  virtual gbtNfgGame GetGame(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  virtual int GetId(void) const = 0;

  virtual bool IsChance(void) const = 0;

  virtual int NumInfosets(void) const = 0;
  //  virtual gbtNfgInfoset NewInfoset(int p_actions) = 0;
  // virtual gbtNfgInfoset GetInfoset(int p_index) const = 0;

  virtual int NumStrategies(void) const = 0;
  virtual gbtNfgAction GetStrategy(int) const = 0;
};

class gbtNfgNullPlayer { };

class gbtNfgPlayer {
private:
  gbtNfgPlayerRep *m_rep;

public:
  gbtNfgPlayer(void) : m_rep(0) { }
  gbtNfgPlayer(gbtNfgPlayerRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgPlayer(const gbtNfgPlayer &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgPlayer() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtNfgPlayer &operator=(const gbtNfgPlayer &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtNfgPlayer &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtNfgPlayer &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtNfgPlayerRep *operator->(void) 
  { if (!m_rep) throw gbtNfgNullPlayer(); return m_rep; }
  const gbtNfgPlayerRep *operator->(void) const 
  { if (!m_rep) throw gbtNfgNullPlayer(); return m_rep; }
  
  gbtNfgPlayerRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

#endif  // PLAYER_H
