//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to extensive form representation class
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

#ifndef EFG_H
#define EFG_H

#include "base/base.h"
#include "math/gnumber.h"
#include "math/gpvector.h"

class gbtEfgSupport;
class gbtNfgGame;
template <class T> class gbtBehavProfile;
template <class T> class gbtMixedProfile;
template <class T> class gbtPureBehavProfile;

#include "outcome.h"
#include "infoset.h"
#include "player.h"
#include "node.h"

class gbtEfgGameBase;
class gbtEfgNodeBase;

//
// Exception classes for the various bad stuff that can happen
//
class gbtEfgException : public gbtException {
public:
  virtual ~gbtEfgException() { }
  gbtText Description(void) const    { return "Error in gbtEfgGame"; }
};

class gbtEfgNullObject : public gbtEfgException {
public:
  virtual ~gbtEfgNullObject() { }
};

class gbtEfgGameMismatch : public gbtEfgException {
public:
  virtual ~gbtEfgGameMismatch() { }
};

class gbtEfgNonterminalNode : public gbtEfgException {
public:
  virtual ~gbtEfgNonterminalNode() { }
};

class gbtEfgGameRep : public gbtGameObject {
friend class EfgFileReader;
friend class EfgFile;
friend class gbtNfgGame;
friend class gbtBehavProfile<double>;
friend class gbtBehavProfile<gbtRational>;
friend class gbtBehavProfile<gbtNumber>;
friend class gbtEfgGame;
friend void SetEfg(gbtNfgGame, gbtEfgGame);
public:
  // Formerly the copy constructor
  virtual gbtEfgGame Copy(gbtEfgNode = gbtEfgNode(0)) const = 0;
  
  // TITLE ACCESS AND MANIPULATION
  virtual void SetLabel(const gbtText &s) = 0;
  virtual gbtText GetLabel(void) const = 0;
  
  virtual void SetComment(const gbtText &) = 0;
  virtual gbtText GetComment(void) const = 0;

  // WRITING DATA FILES
  virtual void WriteEfg(gbtOutput &p_file) const = 0;

  // DATA ACCESS -- GENERAL INFORMATION
  virtual bool IsConstSum(void) const = 0; 
  virtual bool IsPerfectRecall(void) const = 0;
  virtual bool IsPerfectRecall(gbtEfgInfoset &, gbtEfgInfoset &) const = 0;
  virtual long RevisionNumber(void) const = 0;
  virtual gbtNumber MinPayoff(int pl = 0) const = 0;
  virtual gbtNumber MaxPayoff(int pl = 0) const = 0;
 
  // DATA ACCESS -- NODES
  virtual int NumNodes(void) const = 0;
  virtual gbtEfgNode GetRoot(void) const = 0;

  // DATA ACCESS -- PLAYERS
  virtual int NumPlayers(void) const = 0;
  virtual gbtEfgPlayer GetChance(void) const = 0;
  virtual gbtEfgPlayer NewPlayer(void) = 0;
  virtual gbtEfgPlayer GetPlayer(int index) const = 0;

  // DATA ACCESS -- OUTCOMES
  virtual int NumOutcomes(void) const = 0;
  virtual gbtEfgOutcome GetOutcome(int p_id) const = 0;
  virtual gbtEfgOutcome NewOutcome(void) = 0;

  // DATA ACCESS -- SUPPORTS
  virtual gbtEfgSupport NewSupport(void) const = 0;

  // EDITING OPERATIONS
  virtual void DeleteEmptyInfosets(void) = 0;

  virtual gbtEfgNode CopyTree(gbtEfgNode src, gbtEfgNode dest) = 0;
  virtual gbtEfgNode MoveTree(gbtEfgNode src, gbtEfgNode dest) = 0;

  virtual gbtEfgAction InsertAction(gbtEfgInfoset) = 0;
  virtual gbtEfgAction InsertAction(gbtEfgInfoset, const gbtEfgAction &at) = 0;

  virtual void SetChanceProb(gbtEfgInfoset, int, const gbtNumber &) = 0;

  virtual void MarkSubgames(void) = 0;
  virtual bool MarkSubgame(gbtEfgNode) = 0;
  virtual void UnmarkSubgame(gbtEfgNode) = 0;
  virtual void UnmarkSubgames(gbtEfgNode) = 0;

  virtual int ProfileLength(void) const = 0;
  virtual int TotalNumInfosets(void) const = 0;

  virtual gbtArray<int> NumInfosets(void) const = 0;  // Does not include chance infosets
  virtual int NumPlayerInfosets(void) const = 0;
  virtual gbtPVector<int> NumActions(void) const = 0;
  virtual int NumPlayerActions(void) const = 0;
  virtual gbtPVector<int> NumMembers(void) const = 0;
  
  // COMPUTING VALUES OF PROFILES
  virtual void Payoff(const gbtPVector<int> &profile,
		      gbtVector<gbtNumber> &payoff) const = 0;
  virtual void Payoff(const gbtArray<gbtArray<int> > &profile,
		      gbtArray<gbtNumber> &payoff) const = 0;

  virtual void InfosetProbs(const gbtPVector<int> &profile,
			    gbtPVector<gbtNumber> &prob) const = 0;
   
  virtual gbtNfgGame GetReducedNfg(void) const = 0;
};


class gbtEfgNullGame { };

class gbtEfgGame {
private:
  gbtEfgGameRep *m_rep;

public:
  gbtEfgGame(void) : m_rep(0) { }
  gbtEfgGame(gbtEfgGameRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgGame(const gbtEfgGame &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgGame() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtEfgGame &operator=(const gbtEfgGame &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtEfgGame &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtEfgGame &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtEfgGameRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullGame(); return m_rep; }
  const gbtEfgGameRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullGame(); return m_rep; }
  
  gbtEfgGameRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};


gbtEfgGame NewEfg(void);

// Exception thrown by ReadEfg if not valid .efg file
class gbtEfgParserError { };
gbtEfgGame ReadEfg(gbtInput &);

template <class T> class gbtPureBehavProfile   {
  protected:
    gbtEfgGame m_efg;
    gbtArray<gbtArray<gbtEfgAction> *> profile;

    //    void IndPayoff(const Node *n, const int &pl, const T, T &) const;
    // This aims at efficiency, but leads to a problem described in behav.imp

    void Payoff(const gbtEfgNode &n, const T, gbtArray<T> &) const;
    void InfosetProbs(const gbtEfgNode &n, T, gbtPVector<T> &) const;

  public:
    gbtPureBehavProfile(const gbtEfgGame &);
    gbtPureBehavProfile(const gbtPureBehavProfile<T> &);
    ~gbtPureBehavProfile();

    // Operators
    gbtPureBehavProfile<T> &operator=(const gbtPureBehavProfile<T> &);
    T operator()(const gbtEfgAction &) const;

    // Manipulation
    void Set(const gbtEfgAction &);
    //    void Set(const gbtEfgPlayer &, const gbtArray<const Action *> &);

    // Information
    gbtEfgAction GetAction(const gbtEfgInfoset &) const;
    
    const T Payoff(const gbtEfgOutcome &, const int &pl) const;
    const T ChanceProb(const gbtEfgInfoset &, const int &act) const;
    
    const T Payoff(const gbtEfgNode &, const int &pl) const;
  //    T    Payoff(const int &pl) const;
    void Payoff(gbtArray<T> &payoff) const;
    void InfosetProbs(gbtPVector<T> &prob) const;
    gbtEfgGame GetGame(void) const   { return m_efg; }
};


#include "efgutils.h"

//
// Stuff below here needs to have everything defined before we can
// declare these.
// Hopefully, once things are settled, these can be moved to more
// logical locations.
//
class gbtEfgPlayerIterator {
private:
  int m_index;
  gbtEfgGame m_efg;

public:
  gbtEfgPlayerIterator(const gbtEfgGame &p_efg);
  
  gbtEfgPlayer operator*(void) const;
  gbtEfgPlayerIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};

class gbtEfgInfosetIterator {
private:
  int m_index;
  gbtEfgPlayer m_player;

public:
  gbtEfgInfosetIterator(const gbtEfgPlayer &p_player);
  
  gbtEfgInfoset operator*(void) const;
  gbtEfgInfosetIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};


#endif   // EFG_H
