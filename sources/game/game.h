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
#include "math/gpvector.h"
#include "math/rational.h"

//!
//! Exception thrown when attempting to manipulate an object that has
//! been deleted from its game.
//!
class gbtGameDeletedException : public gbtException {
public:
  virtual ~gbtGameDeletedException() { }
  std::string GetDescription(void) const  { return "Game object deleted"; }
};

//!
//! Exception thrown when attempting to dereference a null object
//! (via its handle class)
//!
class gbtGameNullException : public gbtException {
public:
  virtual ~gbtGameNullException() { }
  std::string GetDescription(void) const { return "Accessing a null object"; }
};

//!
//! Exception thrown when an operation is not defined for the implementation
//! of the game interface.
//!
class gbtGameUndefinedException : public gbtException {
public:
  virtual ~gbtGameUndefinedException() { }
  std::string GetDescription(void) const { return "Undefined operation"; }
};

//!
//! Exception thrown when attempting to operate between two mismatched
//! game objects.
//!
class gbtGameMismatchException : public gbtException {
public:
  virtual ~gbtGameMismatchException() { }
  std::string GetDescription(void) const { return "Mismatched operands"; }
};

template <class T> class gbtGameObjectHandle {
private:
  T *m_rep;

public:
  gbtGameObjectHandle(void) : m_rep(0) { }
  gbtGameObjectHandle(T *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtGameObjectHandle(const gbtGameObjectHandle<T> &p_handle)
    : m_rep(p_handle.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtGameObjectHandle() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtGameObjectHandle<T> &operator=(const gbtGameObjectHandle<T> &p_handle) {
    if (this != &p_handle) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_handle.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtGameObjectHandle<T> &p_handle) const
  { return (m_rep == p_handle.m_rep); }
  bool operator!=(const gbtGameObjectHandle<T> &p_handle) const
  { return (m_rep != p_handle.m_rep); }

  T *operator->(void) 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  const T *operator->(void) const 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  
  T *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

template <class T> class gbtGameSingleHandle {
private:
  T *m_rep;

public:
  gbtGameSingleHandle(void) : m_rep(0) { }
  gbtGameSingleHandle(T *p_rep)
    : m_rep(p_rep) { }
  gbtGameSingleHandle(const gbtGameSingleHandle<T> &p_handle)
    : m_rep(p_handle.m_rep->Copy()) { }
  ~gbtGameSingleHandle() { delete m_rep; }

  gbtGameSingleHandle<T> &operator=(const gbtGameSingleHandle<T> &p_handle) {
    if (this != &p_handle) {
      if (m_rep) delete m_rep;
      m_rep = p_handle.m_rep->Copy();
    }
    return *this;
  }

  T *operator->(void) 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  const T *operator->(void) const 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  
  T *Get(void) const { return m_rep; }
};


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

class gbtGamePlayerRep;
typedef gbtGameObjectHandle<gbtGamePlayerRep> gbtGamePlayer;

class gbtGameOutcomeRep;
typedef gbtGameObjectHandle<gbtGameOutcomeRep> gbtGameOutcome;

class gbtGameNodeRep;
typedef gbtGameObjectHandle<gbtGameNodeRep> gbtGameNode;

class gbtNfgContingencyRep;
typedef gbtGameSingleHandle<gbtNfgContingencyRep> gbtNfgContingency;

template <class T> class gbtBehavProfile;
template <class T> class gbtMixedProfile;
class gbtPureBehavProfile;

//!
//! An abstract representation of a game.  Implementations of this
//! interface may define the extensive form features, normal form
//! features, or both.  Member functions corresponding to unimplemented
//! or unsupported representations should throw a gbtGameUndefined
//! exception.
//!
class gbtGameRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameRep>;

public:
  //!
  //! @name Manipulation of titles and comments
  //!
  //@{
  /// Set the text label associated with the game.
  virtual void SetLabel(const std::string &s) = 0;
  /// Get the text label associated with the game.
  virtual std::string GetLabel(void) const = 0;

  /// Set the comment (a longer text string) associated with the game.
  virtual void SetComment(const std::string &) = 0;
  /// Get the comment (a longer text string) associated with the game.
  virtual std::string GetComment(void) const = 0;
  //@}

  //!
  //! @name General information about the game
  //!
  //@{
  /// Returns true if the game is constant sum.
  virtual bool IsConstSum(void) const = 0; 

  /// Returns true if the game is perfect recall.
  virtual bool IsPerfectRecall(void) const = 0;

  /// Returns the smallest payoff to any player possible in the game.
  virtual gbtRational GetMinPayoff(void) const = 0;

  /// Returns the largest payoff to any player possible in the game.
  virtual gbtRational GetMaxPayoff(void) const = 0;
  //@}

  //!
  //! @name Information about the game tree (if present)
  //!
  //@{
  /// Returns true if the game has a tree representation.
  virtual bool HasTree(void) const = 0;
  /// Returns the root node of the game tree.
  virtual gbtGameNode GetRoot(void) const = 0;
  //}

  //!
  //! @name Information about the game table
  //!
  //@{
  /// Returns an object representing a contingency in the game.
  virtual gbtNfgContingency NewContingency(void) const = 0;
  //@}

  //!
  //! @name Manipulation of players in the game
  //!
  //@{
  /// Returns the number of players in the game.
  virtual int NumPlayers(void) const = 0;
  /// Returns the chance (or nature) player in the game.
  virtual gbtGamePlayer GetChance(void) const = 0;
  /// Creates a new player in the game.
  virtual gbtGamePlayer NewPlayer(void) = 0;
  /// Returns the indexth player in the game.
  virtual gbtGamePlayer GetPlayer(int index) const = 0;
  //@}

  //!
  //! @name Manipulation of outcomes in the game
  //!
  //@{
  /// Returns the number of outcomes in the game.
  virtual int NumOutcomes(void) const = 0;
  /// Creates a new outcome in the game.
  virtual gbtGameOutcome NewOutcome(void) = 0;
  /// Returns the indexth outcome in the game.
  virtual gbtGameOutcome GetOutcome(int index) const = 0;
  //@}

  //!
  //! @name Information about the dimensions of the game
  //!
  //@{
  /// Returns the length of a behavior profile on the game.
  virtual int BehaviorProfileLength(void) const = 0;
  /// Returns the number of information sets, by player.
  virtual gbtArray<int> NumInfosets(void) const = 0;
  /// Returns the number of actions, by information set.
  virtual gbtPVector<int> NumActions(void) const = 0;
  /// Returns the number of information set members, by information set.
  virtual gbtPVector<int> NumMembers(void) const = 0;
  
  /// Returns the length of a (mixed) strategy profile on the game.
  virtual int StrategyProfileLength(void) const = 0;
  /// Returns the number of strategies, by player.
  virtual gbtArray<int> NumStrategies(void) const = 0;
  //@}

  //!
  //! @name Creating strategy profiles on the game
  //!
  //@{
  /// Create a new mixed strategy profile in double precision
  virtual gbtMixedProfile<double> NewMixedProfile(double) const = 0;
  /// Create a new mixed strategy profile in rational precision
  virtual gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const = 0;

  /// Create a new behavior profile in double precision
  virtual gbtBehavProfile<double> NewBehavProfile(double) const = 0;
  /// Create a new behavior profile in rational precision
  virtual gbtBehavProfile<gbtRational> NewBehavProfile(const gbtRational &) const = 0;
  //@}

  //!
  //! @name Writing data files
  //!
  //@{
  /// Writes the game tree (if defined) to p_file
  virtual void WriteEfg(std::ostream &p_file) const = 0;
  
  /// Writes the normal form to p_file
  virtual void WriteNfg(std::ostream &p_file) const = 0;
  //@}
};

typedef gbtGameObjectHandle<gbtGameRep> gbtGame;


#include "game-outcome.h"
#include "game-infoset.h"
#include "game-player.h"
#include "game-node.h"
#include "game-strategy.h"
#include "game-contingency.h"

gbtGame NewEfg(void);
gbtGame ReadEfg(std::istream &);

// Exception thrown by ReadEfg if not valid .efg file
class gbtEfgParserException : public gbtException {
public:
  virtual ~gbtEfgParserException() { }
  std::string GetDescription(void) const { return "Not a valid .efg file"; }
};


gbtGame NewNfg(const gbtArray<int> &);
gbtGame ReadNfg(std::istream &);

// Exception thrown by ReadNfg if not valid .nfg file
class gbtNfgParserException : public gbtException {
public:
  virtual ~gbtNfgParserException() { }
  std::string GetDescription(void) const { return "Not a valid .nfg file"; }
};

#include "efgutils.h"

#include "game-behav-mixed.h"
#include "tree-behav-pure.h"
#include "game-strategy-mixed.h"


#endif   // GAME_H
