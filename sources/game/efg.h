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
template <class T> class BehavProfile;
template <class T> class MixedProfile;
template <class T> class PureBehavProfile;

#include "outcome.h"
#include "infoset.h"
#include "player.h"
#include "node.h"

struct gbt_efg_game_rep;

//
// Exception classes for the various bad stuff that can happen
//
class gbtEfgbtException : public gbtException {
public:
  virtual ~gbtEfgbtException() { }
  gbtText Description(void) const    { return "Error in gbtEfgGame"; }
};

class gbtEfgNullObject : public gbtEfgbtException {
public:
  virtual ~gbtEfgNullObject() { }
};

class gbtEfgGameMismatch : public gbtEfgbtException {
public:
  virtual ~gbtEfgGameMismatch() { }
};

class gbtEfgNonterminalNode : public gbtEfgbtException {
public:
  virtual ~gbtEfgNonterminalNode() { }
};

class gbtEfgGame {
private:
  friend class EfgFileReader;
  friend class EfgFile;
  friend class gbtNfgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend void SetEfg(gbtNfgGame, gbtEfgGame);

protected:
  struct gbt_efg_game_rep *rep;
  
  // this is for use with the copy constructor
  void CopySubtree(gbt_efg_game_rep *,
		   gbt_efg_node_rep *, gbt_efg_node_rep *);

  void CopySubtree(gbt_efg_node_rep *, gbt_efg_node_rep *,
		   gbt_efg_node_rep *);

  gbtEfgOutcome NewOutcome(int index);

  void WriteEfg(gbtOutput &, gbt_efg_node_rep *) const;

  void Payoff(gbt_efg_node_rep *n, gNumber,
	      const gPVector<int> &, gVector<gNumber> &) const;
  void Payoff(gbt_efg_node_rep *n, gNumber,
	      const gbtArray<gbtArray<int> *> &, gbtArray<gNumber> &) const;
  
  void InfosetProbs(gbt_efg_node_rep *n, gNumber,
		    const gPVector<int> &, gPVector<gNumber> &) const;
    

public:
  gbtEfgGame(void);
  gbtEfgGame(const gbtEfgGame &);
  gbtEfgGame(gbt_efg_game_rep *);
  ~gbtEfgGame();

  gbtEfgGame &operator=(const gbtEfgGame &);

  bool operator==(const gbtEfgGame &) const;
  bool operator!=(const gbtEfgGame &) const;

  // Formerly the copy constructor
  gbtEfgGame Copy(gbtEfgNode = gbtEfgNode(0)) const;
  
  // TITLE ACCESS AND MANIPULATION
  void SetLabel(const gbtText &s);
  const gbtText &GetLabel(void) const;
  
  void SetComment(const gbtText &);
  const gbtText &GetComment(void) const;

  // WRITING DATA FILES
  void WriteEfg(gbtOutput &p_file) const;

  // DATA ACCESS -- GENERAL INFORMATION
  bool IsConstSum(void) const; 
  bool IsPerfectRecall(void) const;
  bool IsPerfectRecall(gbtEfgInfoset &, gbtEfgInfoset &) const;
  long RevisionNumber(void) const;
  gNumber MinPayoff(int pl = 0) const;
  gNumber MaxPayoff(int pl = 0) const;
  
  gbtEfgNode GetRoot(void) const;

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const;
  gbtEfgPlayer GetChance(void) const;
  gbtEfgPlayer NewPlayer(void);
  gbtEfgPlayer GetPlayer(int index) const;

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const;
  gbtEfgOutcome GetOutcome(int p_id) const;
  gbtEfgOutcome NewOutcome(void);

  // DATA ACCESS -- SUPPORTS
  gbtEfgSupport NewSupport(void) const;

  // EDITING OPERATIONS
  void DeleteEmptyInfosets(void);

  gbtEfgNode CopyTree(gbtEfgNode src, gbtEfgNode dest);
  gbtEfgNode MoveTree(gbtEfgNode src, gbtEfgNode dest);

  gbtEfgAction InsertAction(gbtEfgInfoset);
  gbtEfgAction InsertAction(gbtEfgInfoset, const gbtEfgAction &at);

  void SetChanceProb(gbtEfgInfoset, int, const gNumber &);

  void MarkSubgames(void);
  bool MarkSubgame(gbtEfgNode);
  void UnmarkSubgame(gbtEfgNode);
  void UnmarkSubgames(gbtEfgNode);

  int ProfileLength(void) const;
  int TotalNumInfosets(void) const;

  gbtArray<int>   NumInfosets(void) const;  // Does not include chance infosets
  int           NumPlayerInfosets(void) const;
  gPVector<int> NumActions(void) const;
  int           NumPlayerActions(void) const;
  gPVector<int> NumMembers(void) const;
  
  // COMPUTING VALUES OF PROFILES
  void Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const;
  void Payoff(const gbtArray<gbtArray<int> *> &profile,
	      gbtArray<gNumber> &payoff) const;

  void InfosetProbs(const gPVector<int> &profile, gPVector<gNumber> &prob) const;
    
  gbtNfgGame GetReducedNfg(void) const;
  bool HasReducedNfg(void) const;
  gbtNfgGame AssociatedAfg(void) const;

  friend gbtNfgGame MakeAfg(const gbtEfgGame &);
};

gbtEfgGame NewEfg(void);
gbtEfgGame ReadEfg(gbtInput &);

template <class T> class PureBehavProfile   {
  protected:
    gbtEfgGame m_efg;
    gbtArray<gbtArray<gbtEfgAction> *> profile;

    //    void IndPayoff(const Node *n, const int &pl, const T, T &) const;
    // This aims at efficiency, but leads to a problem described in behav.imp

    void Payoff(const gbtEfgNode &n, const T, gbtArray<T> &) const;
    void InfosetProbs(const gbtEfgNode &n, T, gPVector<T> &) const;

  public:
    PureBehavProfile(const gbtEfgGame &);
    PureBehavProfile(const PureBehavProfile<T> &);
    ~PureBehavProfile();

    // Operators
    PureBehavProfile<T> &operator=(const PureBehavProfile<T> &);
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
    void InfosetProbs(gPVector<T> &prob) const;
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
