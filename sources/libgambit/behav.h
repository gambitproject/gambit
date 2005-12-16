//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of behavior strategy classes
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

#ifndef BEHAV_H
#define BEHAV_H

#include "gdpvect.h"
#include "efg.h"

class gbtNfgGame;
template <class T> class gbtMixedProfile;
template <class T> class gbtPVector;
template <class T> class gbtRectArray;

//
//  gbtBehavProfile<T> implements a behavior profile on an Efg.  
//
//  The class assumes that the underlying Efg does not change during the 
//  life of the profile, and will not correctly invalidate itself if 
//  the game does change.  
// 
// 

template <class T> class gbtBehavProfile : private gbtDPVector<T>  {
protected:
  const gbtEfgGame *m_efg;
  gbtEfgSupport m_support;
  mutable bool m_cached_data;

  // structures for storing cached data: nodes
  mutable gbtVector<T> m_realizProbs, m_beliefs, m_nvals, m_bvals;
  mutable gbtMatrix<T> m_nodeValues;

  // structures for storing cached data: information sets
  mutable gbtPVector<T> m_infosetValues;

  // structures for storing cached data: actions
  mutable gbtDPVector<T> m_actionValues;   // aka conditional payoffs
  mutable gbtDPVector<T> m_gripe;

  void InitProfile(void);

  //
  // FUNCTIONS FOR DATA ACCESS
  //
  // NOTE: These functions all assume that profile is installed, and that relevant 
  // data has been computed.  
  // Use public versions (GetNodeValue, GetIsetProb, etc) if this is not known.

  const T &RealizProb(const gbtEfgNode &node) const;
  T &RealizProb(const gbtEfgNode &node);

  const T &BeliefProb(const gbtEfgNode &node) const;
  T &BeliefProb(const gbtEfgNode &node);
  
  gbtVector<T> NodeValues(const gbtEfgNode &node) const
    { return m_nodeValues.Row(node->number); }
  const T &NodeValue(const gbtEfgNode &node, int pl) const
    { return m_nodeValues(node->number, pl); }
  T &NodeValue(const gbtEfgNode &node, int pl)
    { return m_nodeValues(node->number, pl); }

  T IsetProb(const gbtEfgInfoset &iset) const;

  const T &IsetValue(const gbtEfgInfoset &iset) const;
  T &IsetValue(const gbtEfgInfoset &iset);

  const T &ActionValue(const gbtEfgAction &act) const 
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetNumber(),
			    act->GetInfoset()->GetNumber(),
			    act->m_number); }
  T &ActionValue(const gbtEfgAction &act)
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetNumber(),
			    act->GetInfoset()->GetNumber(),
			    act->m_number); }
  
  T ActionProb(const gbtEfgAction &act) const;

  const T &Regret(const gbtEfgAction &act) const;
  T &Regret(const gbtEfgAction &);

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES

  void Payoff(gbtEfgNodeRep *, T, int, T &) const;
  T Payoff(gbtEfgOutcomeRep *, int pl) const;
  
  void ComputeSolutionDataPass2(const gbtEfgNode &node) const;
  void ComputeSolutionDataPass1(const gbtEfgNode &node) const;
  void ComputeSolutionData(void) const;

  void BehaviorStrat(const gbtEfgGame &, int, gbtEfgNode &);
  void RealizationProbs(const gbtMixedProfile<T> &, const gbtEfgGame &,
			int pl, const gbtArray<int> &, gbtEfgNode);

public:
  // CONSTRUCTORS, DESTRUCTOR

  gbtBehavProfile(const gbtEfgSupport &);
  gbtBehavProfile(const gbtBehavProfile<T> &);
  gbtBehavProfile(const gbtMixedProfile<T> &);
  virtual ~gbtBehavProfile();
  
  // OPERATOR OVERLOADING

  gbtBehavProfile<T> &operator=(const gbtBehavProfile<T> &);
  inline gbtBehavProfile<T> &operator=(const gbtVector<T> &p)
    { Invalidate(); gbtVector<T>::operator=(p); return *this;}

  bool operator==(const gbtBehavProfile<T> &) const;
  bool operator!=(const gbtBehavProfile<T> &x) const
    { return !(operator==(x)); }  

  // INITIALIZATION, VALIDATION
  inline void Invalidate(void) const {m_cached_data=false;}
  void Centroid(void) const;

  // GENERAL DATA ACCESS

  gbtEfgGame &GetGame(void) const   { return const_cast<gbtEfgGame &>(*m_efg); }
  const gbtEfgSupport &Support(void) const   { return m_support; }
  
  const T &GetRealizProb(const gbtEfgNode &node) const;
  const T &GetBeliefProb(const gbtEfgNode &node) const;
  gbtVector<T> GetNodeValue(const gbtEfgNode &node) const;
  T GetIsetProb(const gbtEfgInfoset &iset) const;
  const T &GetIsetValue(const gbtEfgInfoset &iset) const;
  T GetActionProb(const gbtEfgAction &act) const;
  const T &GetActionValue(const gbtEfgAction &act) const;
  const T &GetRegret(const gbtEfgAction &act) const;

  // COMPUTATION OF INTERESTING QUANTITIES

  T Payoff(int p_player) const;
  gbtDPVector<T> Beliefs(void);
  T LiapValue(void) const;
  T LiapValueOnDefined(void) const;
  T MaxRegret(void);

  bool IsDefinedAt(gbtEfgInfoset p_infoset) const;

  T DiffActionValue(const gbtEfgAction &action, 
		    const gbtEfgAction &oppAction) const;
  T DiffRealizProb(const gbtEfgNode &node, 
		   const gbtEfgAction &oppAction) const;
  T DiffNodeValue(const gbtEfgNode &node, const gbtEfgPlayer &player,
		  const gbtEfgAction &oppAction) const;

  // IMPLEMENTATION OF gbtDPVector OPERATIONS
  // These are reimplemented here to correctly handle invalidation
  // of cached information.
  const T &operator()(int a, int b, int c) const
    { return gbtDPVector<T>::operator()(a, b, c); }
  T &operator()(int a, int b, int c) 
    { Invalidate();  return gbtDPVector<T>::operator()(a, b, c); }
  const T &operator[](int a) const
    { return gbtArray<T>::operator[](a); }
  T &operator[](int a)
    { Invalidate();  return gbtArray<T>::operator[](a); }

  gbtBehavProfile<T> &operator=(const T &x)  
    { Invalidate();  gbtDPVector<T>::operator=(x);  return *this; }

  bool operator==(const gbtDPVector<T> &x) const
    { return gbtDPVector<T>::operator==(x); }
  bool operator!=(const gbtDPVector<T> &x) const
    { return gbtDPVector<T>::operator!=(x); }

  gbtBehavProfile<T> &operator+=(const gbtBehavProfile<T> &x)
    { Invalidate();  gbtDPVector<T>::operator+=(x);  return *this; }
  gbtBehavProfile<T> &operator+=(const gbtDPVector<T> &x)
    { Invalidate();  gbtDPVector<T>::operator+=(x);  return *this; }
  gbtBehavProfile<T> &operator-=(const gbtBehavProfile<T> &x)
    { Invalidate();  gbtDPVector<T>::operator-=(x);  return *this; }
  gbtBehavProfile<T> &operator*=(const T &x)
    { Invalidate();  gbtDPVector<T>::operator*=(x);  return *this; }

  int Length(void) const
    { return gbtArray<T>::Length(); }
  const gbtArray<int> &Lengths(void) const
    { return gbtPVector<T>::Lengths(); }
  int First(void) const { return gbtArray<T>::First(); }
  int Last(void) const { return gbtArray<T>::Last(); }

  const gbtPVector<T> &GetPVector(void) const { return *this; }
  const gbtDPVector<T> &GetDPVector(void) const { return *this; }
  gbtDPVector<T> &GetDPVector(void) { Invalidate(); return *this; }
};


#endif   // BEHAV_H
