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

#include "game.h"
#include "efgsupport.h"

template <class T> class gbtMixedProfile;

//
//  gbtBehavProfile<T> implements a behavior profile on a game.
//
//  The class assumes that the underlying game does not change during the 
//  life of the profile, and will not correctly invalidate itself if 
//  the game does change.  
// 
//  The BehavSolution class should be used for interactive use, where 
//  the game payoffs or probabilities may change.  
// 

template <class T> 
class gbtBehavProfile : private gbtDPVector<T>, public gbtConstEfgRep {
protected:
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

  void InitPayoffs(void) const;
  void InitProfile(void);

  //
  // FUNCTIONS FOR DATA ACCESS
  //
  // NOTE: These functions all assume the cached data is up-to-date.
  // Use public versions (GetNodeValue, GetIsetProb, etc) if this is not known.
  //
  const T &RealizProb(const gbtGameNode &node) const;
  T &RealizProb(const gbtGameNode &node);

  const T &BeliefProb(const gbtGameNode &node) const;
  T &BeliefProb(const gbtGameNode &node);
  
  gbtVector<T> NodeValues(const gbtGameNode &node) const
    { return m_nodeValues.Row(node->GetId()); }
  const T &NodeValue(const gbtGameNode &node, int pl) const
    { return m_nodeValues(node->GetId(), pl); }
  T &NodeValue(const gbtGameNode &node, int pl)
    { return m_nodeValues(node->GetId(), pl); }

  T IsetProb(const gbtGameInfoset &iset) const;

  const T &IsetValue(const gbtGameInfoset &iset) const;
  T &IsetValue(const gbtGameInfoset &iset);

  const T &ActionValue(const gbtGameAction &act) const 
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetId(),
			    act->GetInfoset()->GetId(),
			    act->GetId()); }
  T &ActionValue(const gbtGameAction &act)
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetId(),
			    act->GetInfoset()->GetId(),
			    act->GetId()); }
  
  T ActionProb(const gbtGameAction &act) const;

  const T &Regret(const gbtGameAction &act) const;
  T &Regret(const gbtGameAction &);

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES

  void Payoff(const gbtGameNode &, T, int, T &) const;
  T Payoff(const gbtGameOutcome &, int pl) const;
  
  void ComputeSolutionDataPass2(const gbtGameNode &node);
  void ComputeSolutionDataPass1(const gbtGameNode &node);
  void ComputeSolutionData(void);

  void BehaviorStrat(const gbtGame &, int, const gbtGameNode &);
  void RealizationProbs(const gbtMixedProfile<T> &,
			int pl, const gbtArray<int> &, const gbtGameNode &);

public:
  class BadStuff : public gbtException  {
  public:
    virtual ~BadStuff();
    gbtText Description(void) const;
  };

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
  virtual bool IsAssessment(void) const { return false; }
  void Centroid(void) const;

  // GENERAL DATA ACCESS

  gbtGame GetGame(void) const   { return m_support.GetTree(); }
  const gbtEfgSupport &GetSupport(void) const   { return m_support; }
  
  const T &GetRealizProb(const gbtGameNode &node);
  const T &GetBeliefProb(const gbtGameNode &node);
  gbtVector<T> GetNodeValue(const gbtGameNode &node);
  T GetIsetProb(const gbtGameInfoset &iset);
  const T &GetIsetValue(const gbtGameInfoset &iset);
  T GetActionProb(const gbtGameAction &act) const;
  const T &GetActionValue(const gbtGameAction &act) const;
  const T &GetRegret(const gbtGameAction &act);

  // COMPUTATION OF INTERESTING QUANTITIES

  T Payoff(int p_player) const;
  gbtDPVector<T> Beliefs(void);
  T LiapValue(bool p_penalty = true);
  T QreValue(const gbtVector<T> &lambda, bool &);
  T MaxRegret(void);

  T DiffActionValue(const gbtGameAction &action, 
		    const gbtGameAction &oppAction) const;
  T DiffRealizProb(const gbtGameNode &node,
		   const gbtGameAction &oppAction) const;
  T DiffNodeValue(const gbtGameNode &node, const gbtGamePlayer &player,
		  const gbtGameAction &oppAction) const;

  void Dump(gbtOutput &) const;

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

  // IMPLEMENTATION OF gbtGameObject INTERFACE
  gbtText GetLabel(void) const { return ""; }
  void SetLabel(const gbtText &) { }

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return m_support.IsTree(); }
  bool IsMatrix(void) const { return m_support.IsMatrix(); }

  gbtText GetComment(void) const { return ""; }

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const { return m_support.NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const { return m_support.GetPlayer(index); }

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const { return m_support.NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_support.GetOutcome(index); }

  bool IsConstSum(void) const { return m_support.IsConstSum(); }
  gbtNumber MaxPayoff(int pl = 0) const { return m_support.MaxPayoff(pl); }
  gbtNumber MinPayoff(int pl = 0) const { return m_support.MinPayoff(pl); }

  // IMPLEMENTATION OF gbtConstEfgRep INTERFACE

  // DATA ACCESS -- GENERAL
  bool IsPerfectRecall(void) const { return m_support.IsPerfectRecall(); }

  // DATA ACCESS -- PLAYERS
  gbtGamePlayer GetChance(void) const { return m_support.GetChance(); }

  // DATA ACCESS -- NODES
  int NumNodes(void) const { return m_support.NumNodes(); }
  gbtGameNode GetRoot(void) const { return m_support.GetRoot(); }

  // DATA ACCESS -- ACTIONS
  gbtPVector<int> NumActions(void) const { return m_support.NumActions(); }
  int BehavProfileLength(void) const { return m_support.BehavProfileLength(); }

  // DATA ACCESS -- INFORMATION SETS
  int TotalNumInfosets(void) const { return m_support.TotalNumInfosets(); }
  gbtArray<int> NumInfosets(void) const { return m_support.NumInfosets(); }
  int NumPlayerInfosets(void) const { return m_support.NumPlayerInfosets(); }
  int NumPlayerActions(void) const { return m_support.NumPlayerActions(); }
  gbtPVector<int> NumMembers(void) const { return m_support.NumMembers(); }

  // DATA ACCESS -- SUPPORTS
  gbtEfgSupport NewEfgSupport(void) const { return m_support.NewEfgSupport(); }

  operator gbtMixedProfile<T>(void) const;
};


//
// Behavioral assessment class.
// Allows for explicit storage and manipulation of a belief system in
// addition to a profile of behavioral strategies
//

template <class T> class gbtBehavAssessment : public gbtBehavProfile<T> {
protected:
  gbtDPVector<T> m_beliefs;
  
  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES
  void CondPayoff(const gbtGameNode &, T,
		  gbtPVector<T> &, gbtDPVector<T> &) const;
  
public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  gbtBehavAssessment(const gbtEfgSupport &);
  gbtBehavAssessment(const gbtBehavProfile<T> &);
  gbtBehavAssessment(const gbtBehavAssessment<T> &);
  virtual ~gbtBehavAssessment();
  
  gbtBehavAssessment<T> &operator=(const gbtBehavAssessment<T> &);
  
  // GENERAL DATA ACCESS
  bool IsAssessment(void) const { return true; }
  
  // ACCESS AND MANIPULATION OF BELIEFS
  gbtDPVector<T> Beliefs(void) const;
  gbtDPVector<T> &Beliefs(void);
  
  void CondPayoff(gbtDPVector<T> &p_payoff, gbtPVector<T> &p_probs) const;
  
  // OUTPUT
  void Dump(gbtOutput &) const;
};


#ifndef __BORLANDC__
template <class T> gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<T> &);
template <class T> gbtOutput &operator<<(gbtOutput &, const gbtBehavAssessment<T> &);
#endif

#endif   // BEHAV_H
