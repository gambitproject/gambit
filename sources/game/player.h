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

struct gbtEfgPlayerBase;
class gbtEfgGame;
class gbtEfgInfoset;

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

struct gbt_nfg_player_rep;
class gbtNfgGame;
class gbtNfgAction;

class gbtNfgPlayer : public gbtGamePlayer {
friend class gbtNfgGame;
friend class gbtNfgOutcome;
protected:
  struct gbt_nfg_player_rep *rep;

public:
  gbtNfgPlayer(void);
  gbtNfgPlayer(gbt_nfg_player_rep *);
  gbtNfgPlayer(const gbtNfgPlayer &);
  ~gbtNfgPlayer();

  gbtNfgPlayer &operator=(const gbtNfgPlayer &);

  bool operator==(const gbtNfgPlayer &) const;
  bool operator!=(const gbtNfgPlayer &) const;

  bool IsNull(void) const;
  bool IsDeleted(void) const;

  gbtNfgGame GetGame(void) const;
  gbtText GetLabel(void) const;
  void SetLabel(const gbtText &);
  int GetId(void) const;

  int NumInfosets(void) const { return 1; }

  int NumStrategies(void) const;
  gbtNfgAction GetStrategy(int) const;
};

#endif  // PLAYER_H
