//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to game representation library
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

#ifndef GAME_H
#define GAME_H

#include "base/base.h"
#include "math/gmath.h"

//
// A base class for all game object types, providing interfaces common
// to all.
//
class gbtGameObject {
protected:
  int m_refCount;

  void Reference(void) { m_refCount++; }
  bool Dereference(void) { return (--m_refCount == 0); }

public:
  gbtGameObject(void) : m_refCount(1) { }
  virtual ~gbtGameObject() { }
};

//
// Exception classes for the various bad stuff that can happen
//
class gbtGameException : public gbtException {
public:
  virtual ~gbtGameException() { }
  gbtText Description(void) const    { return "Error in gbtGame"; }
};

class gbtGameNullObject : public gbtGameException {
public:
  virtual ~gbtGameNullObject() { }
};

class gbtGameObjectDeleted : public gbtGameException {
public:
  virtual ~gbtGameObjectDeleted() { }
};

class gbtGameUndefinedOperation : public gbtGameException {
public:
  virtual ~gbtGameUndefinedOperation() { }
};

//
// Forward declarations of game objects
//
class gbtGameAction;
class gbtGameInfoset;
class gbtGameStrategy;
class gbtGamePlayer;
class gbtGameNode;
class gbtGameOutcome;

class gbtEfgSupport;
class gbtNfgSupport;

class gbtNfgContingency;

template <class T> class gbtMixedProfile;
template <class T> class gbtBehavProfile;

class gbtConstGameRep : public gbtGameObject {
public:
  // DATA ACCESS -- GENERAL
  virtual bool IsTree(void) const = 0;
  virtual bool IsMatrix(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;
  virtual gbtText GetComment(void) const = 0;

  // DATA ACCESS -- PLAYERS
  virtual int NumPlayers(void) const = 0;
  virtual gbtGamePlayer GetPlayer(int index) const = 0;

  // DATA ACCESS -- OUTCOMES
  virtual int NumOutcomes(void) const = 0;
  virtual gbtGameOutcome GetOutcome(int p_id) const = 0;
  virtual bool IsConstSum(void) const = 0; 
  virtual gbtNumber GetMinPayoff(void) const = 0;
  virtual gbtNumber GetMaxPayoff(void) const = 0;
};

class gbtConstNfgRep : public virtual gbtConstGameRep {
  friend class gbtNfgGame;
public:
  // DATA ACCESS -- STRATEGIES
  virtual gbtArray<int> NumStrategies(void) const = 0; 
  virtual int MixedProfileLength(void) const = 0;

  virtual gbtNfgContingency NewContingency(void) const = 0;

  // DATA ACCESS -- SUPPORTS
  virtual gbtNfgSupport NewNfgSupport(void) const = 0;

  // DATA ACCESS -- PROFILES
  // These could be declared template, but aren't, for portability
  virtual gbtMixedProfile<double> NewMixedProfile(double) const = 0;
  virtual gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const = 0;
  virtual gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const = 0;
};

class gbtNfgGame {
private:
  gbtConstNfgRep *m_rep;

public:
  gbtNfgGame(void) : m_rep(0) { }
  gbtNfgGame(gbtConstNfgRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgGame(const gbtNfgGame &p_nfg)
    : m_rep(p_nfg.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgGame() { if (m_rep && m_rep->Dereference()) delete m_rep; }
  
  gbtNfgGame &operator=(const gbtNfgGame &p_nfg) {
    if (this != &p_nfg) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_nfg.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtNfgGame &p_nfg) const
  { return (m_rep == p_nfg.m_rep); }
  bool operator!=(const gbtNfgGame &p_nfg) const
  { return (m_rep != p_nfg.m_rep); }
  
  gbtConstNfgRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtConstNfgRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }

  gbtConstNfgRep *Get(void) const { return m_rep; }
  
  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

class gbtConstEfgRep : public virtual gbtConstGameRep {
public:
  // DATA ACCESS -- GENERAL
  virtual bool IsPerfectRecall(void) const = 0;

  // DATA ACCESS -- PLAYERS
  virtual gbtGamePlayer GetChance(void) const = 0;

  // DATA ACCESS -- NODES
  virtual int NumNodes(void) const = 0;
  virtual gbtGameNode GetRoot(void) const = 0;

  // DATA ACCESS -- ACTIONS
  virtual gbtPVector<int> NumActions(void) const = 0;
  virtual int BehavProfileLength(void) const = 0;

  // DATA ACCESS -- INFORMATION SETS
  virtual int TotalNumInfosets(void) const = 0;
  // The number of information sets in the game, by player
  // Does not include any chance information sets
  virtual gbtArray<int> NumInfosets(void) const = 0; 
  virtual int NumPlayerInfosets(void) const = 0;
  virtual int NumPlayerActions(void) const = 0;
  virtual gbtPVector<int> NumMembers(void) const = 0;

  // DATA ACCESS -- SUPPORTS
  virtual gbtEfgSupport NewEfgSupport(void) const = 0;

  // DATA ACCESS -- PROFILES
  // These could be declared template, but aren't, for portability
  virtual gbtBehavProfile<double> NewBehavProfile(double) const = 0;
  virtual gbtBehavProfile<gbtRational> NewBehavProfile(const gbtRational &) const = 0;
  virtual gbtBehavProfile<gbtNumber> NewBehavProfile(const gbtNumber &) const = 0;
};

class gbtGameRep : public gbtConstNfgRep, public gbtConstEfgRep {
friend class gbtGame;
public:
  // Formerly the copy constructor
  virtual gbtGame Copy(gbtGameNode) const = 0;
  
  // TITLE ACCESS AND MANIPULATION
  virtual void SetLabel(const gbtText &) = 0;
  virtual void SetComment(const gbtText &) = 0;

  // WRITING DATA FILES
  virtual void WriteEfg(gbtOutput &p_file) const = 0;
  virtual void WriteNfg(gbtOutput &p_file) const = 0;

  // DATA ACCESS -- GENERAL INFORMATION
  virtual long RevisionNumber(void) const = 0;
 
  // DATA ACCESS -- PLAYERS
  virtual gbtGamePlayer NewPlayer(void) = 0;

  // DATA ACCESS -- OUTCOMES
  virtual gbtGameOutcome NewOutcome(void) = 0;

  // EDITING OPERATIONS
  virtual void DeleteEmptyInfosets(void) = 0;

  virtual gbtGameNode CopyTree(gbtGameNode src, gbtGameNode dest) = 0;
  virtual gbtGameNode MoveTree(gbtGameNode src, gbtGameNode dest) = 0;

  virtual gbtGameAction InsertAction(gbtGameInfoset) = 0;
  virtual gbtGameAction InsertAction(gbtGameInfoset, const gbtGameAction &at) = 0;

  virtual void SetChanceProb(gbtGameInfoset, int, const gbtNumber &) = 0;

  virtual void MarkSubgames(void) = 0;
  virtual bool MarkSubgame(gbtGameNode) = 0;
  virtual void UnmarkSubgame(gbtGameNode) = 0;
  virtual void UnmarkSubgames(gbtGameNode) = 0;
};


class gbtGame {
private:
  gbtGameRep *m_rep;

public:
  gbtGame(void) : m_rep(0) { }
  gbtGame(gbtGameRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtGame(const gbtGame &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtGame() { if (m_rep && m_rep->Dereference()) delete m_rep; }
  
  gbtGame &operator=(const gbtGame &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtGame &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtGame &p_player) const
  { return (m_rep != p_player.m_rep); }
  
  gbtGameRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtGameRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }

  gbtGameRep *Get(void) const { return m_rep; }
  
  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};


gbtGame NewEfg(void);
gbtGame NewNfg(const gbtArray<int> &);

// Exception thrown by ReadEfg if not valid .efg file
class gbtEfgParserError { };
gbtGame ReadEfg(gbtInput &);

// Exception thrown by ReadNfg if not valid .nfg file
class gbtNfgParserError { };
gbtGame ReadNfg(gbtInput &);


#include "outcome.h"
#include "player.h"
#include "infoset.h"
#include "node.h"

#include "efgutils.h"

//
// Stuff below here needs to have everything defined before we can
// declare these.
// Hopefully, once things are settled, these can be moved to more
// logical locations.
//
class gbtGamePlayerIterator {
private:
  int m_index;
  gbtGame m_efg;

public:
  gbtGamePlayerIterator(const gbtGame &p_efg);

  gbtGamePlayer operator*(void) const;
  gbtGamePlayerIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};

class gbtGameInfosetIterator {
private:
  int m_index;
  gbtGamePlayer m_player;

public:
  gbtGameInfosetIterator(const gbtGamePlayer &p_player);
  
  gbtGameInfoset operator*(void) const;
  gbtGameInfosetIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};

#endif   // GAME_H
