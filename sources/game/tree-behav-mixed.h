//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of behavior profile class for game trees
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

#ifndef TREE_BEHAV_MIXED_H
#define TREE_BEHAV_MIXED_H

#include "tree-game.h"
#include "game-behav-mixed.h"
#include "tree-strategy-mixed.h"

template <class T> 
class gbtTreeBehavProfileRep : public gbtBehavProfileRep<T> {
public:
  int m_refCount;
  gbtTreeGameRep *m_efg;
  gbtDPVector<T> m_profile;
  mutable bool m_cachedData;

  // structures for storing cached data: nodes
  mutable gbtVector<T> m_realizProbs, m_beliefs, m_nvals, m_bvals;
  mutable gbtMatrix<T> m_nodeValues;

  // structures for storing cached data: information sets
  mutable gbtPVector<T> m_infosetValues;

  // structures for storing cached data: actions
  mutable gbtDPVector<T> m_actionValues;   // aka conditional payoffs


  /// @name Constructors and destructor
  //@{
  gbtTreeBehavProfileRep(const gbtTreeGameRep *);
  gbtTreeBehavProfileRep(const gbtTreeBehavProfileRep<T> &);
  gbtTreeBehavProfileRep(const gbtTreeMixedProfileRep<T> *);
  virtual ~gbtTreeBehavProfileRep();

  gbtBehavProfileRep<T> *Copy(void) const;
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  //@}

  /// @name General information about the profile
  //@{
  gbtGame GetGame(void) const;
  int BehaviorProfileLength(void) const  { return m_profile.Length(); }
  bool IsDeleted(void) const { return false; }

  bool operator==(const gbtBehavProfileRep<T> &) const;
  //@}
  
  /// @name Accessing the profile probabilities
  //@{
  T GetActionProb(const gbtGameAction &act) const;
  void SetActionProb(const gbtGameAction &, const T &);
  void SetCentroid(void);
  //@}

  /// @name Vector-like operations for accessing probabilities
  //@{
  const T &operator[](int a) const
    { return m_profile[a]; }
  T &operator[](int a)
    { m_cachedData = false;  return m_profile[a]; }
  const T &operator()(int a, int b, int c) const 
    { return m_profile(a, b, c); }
  T &operator()(int a, int b, int c) 
    { m_cachedData = false;  return m_profile(a, b, c); }
  operator gbtVector<T>(void) const { return m_profile; }
  //@}

  /// @name Payoff information
  //@{
  T GetPayoff(const gbtGamePlayer &) const;
  T GetNodeValue(const gbtGameNode &node, const gbtGamePlayer &) const;
  T GetInfosetValue(const gbtGameInfoset &) const;
  T GetActionValue(const gbtGameAction &) const;

  T DiffActionValue(const gbtGameAction &action, 
		    const gbtGameAction &oppAction) const;
  T DiffRealizProb(const gbtGameNode &node,
		   const gbtGameAction &oppAction) const;
  T DiffNodeValue(const gbtGameNode &node, const gbtGamePlayer &player,
		  const gbtGameAction &oppAction) const;

  T GetLiapValue(bool p_penalty) const;
  //@}
  
  /// @name Probability information
  //@{
  T GetRealizProb(const gbtGameNode &) const;
  T GetBeliefProb(const gbtGameNode &) const;
  T GetInfosetProb(const gbtGameInfoset &) const;
  //@}

  /// @name Functions private to the implementation
  //@{
  void GetPayoff(gbtTreeNodeRep *, T, gbtTreePlayerRep *, T &) const;
  
  void ComputeSolutionDataPass2(gbtTreeNodeRep *) const;
  void ComputeSolutionDataPass1(gbtTreeNodeRep *) const;
  void ComputeSolutionData(void) const;

  void BehaviorStrat(int, gbtTreeNodeRep *);
  void RealizationProbs(const gbtTreeMixedProfileRep<T> &, 
			int pl, const gbtArray<int> &, gbtTreeNodeRep *);
  //@}
};

#endif  // TREE_BEHAV_MIXED_H
