//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of behavior profiles for tree-represented extensive forms
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef BEHAVTREE_H
#define BEHAVTREE_H

#include "behav.h"
#include "gamebase.h"    // currently for gbtEfgSupportBase

template <class T> class gbtMixedProfileTree;

template <class T> 
class gbtBehavProfileTree : public gbtBehavProfileRep<T> {
public:
  gbtDPVector<T> m_profile;
  gbtEfgSupportBase *m_support;
  mutable bool m_cachedData;

  // structures for storing cached data: nodes
  mutable gbtVector<T> m_realizProbs, m_beliefs, m_nvals, m_bvals;
  mutable gbtMatrix<T> m_nodeValues;

  // structures for storing cached data: information sets
  mutable gbtPVector<T> m_infosetValues;

  // structures for storing cached data: actions
  mutable gbtDPVector<T> m_actionValues;   // aka conditional payoffs
  mutable gbtDPVector<T> m_gripe;

  //
  // FUNCTIONS FOR DATA ACCESS
  //
  // NOTE: These functions all assume the cached data is up-to-date.
  // Use public versions (GetNodeValue, GetIsetProb, etc) if this is not known.
  
  T &RealizProb(const gbtGameNode &node) const;
  T &RealizProb(const gbtGameNode &node);

  T &BeliefProb(const gbtGameNode &node) const;
  T &BeliefProb(const gbtGameNode &node);
  
  gbtVector<T> NodeValues(const gbtGameNode &node) const
    { return m_nodeValues.Row(node->GetId()); }
  T &NodeValue(const gbtGameNode &node, int pl) const
    { return m_nodeValues(node->GetId(), pl); }
  T &NodeValue(const gbtGameNode &node, int pl)
    { return m_nodeValues(node->GetId(), pl); }

  T IsetProb(const gbtGameInfoset &iset) const;

  T &IsetValue(const gbtGameInfoset &iset) const;
  T &IsetValue(const gbtGameInfoset &iset);

  T &ActionValue(const gbtGameAction &act) const 
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetId(),
			    act->GetInfoset()->GetId(),
			    act->GetId()); }
  T &ActionValue(const gbtGameAction &act)
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetId(),
			    act->GetInfoset()->GetId(),
			    act->GetId()); }
  
  T ActionProb(const gbtGameAction &act) const;

  T &Regret(const gbtGameAction &act) const;
  T &Regret(const gbtGameAction &);

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES

  void Payoff(const gbtGameNode &, T, int, T &) const;
  T Payoff(const gbtGameOutcome &, int pl) const;
  
  void ComputeSolutionDataPass2(const gbtGameNode &node) const;
  void ComputeSolutionDataPass1(const gbtGameNode &node) const;
  void ComputeSolutionData(void) const;

  void BehaviorStrat(int, const gbtGameNode &);
  void RealizationProbs(const gbtMixedProfileTree<T> &,
			int pl, const gbtArray<int> &, const gbtGameNode &);

  // CONSTRUCTORS, DESTRUCTOR
  gbtBehavProfileTree(const gbtEfgSupportBase &);
  gbtBehavProfileTree(const gbtBehavProfileTree<T> &);
  gbtBehavProfileTree(const gbtMixedProfileTree<T> &);
  virtual ~gbtBehavProfileTree();

  gbtBehavProfileRep<T> *Copy(void) const;
  
  // OPERATOR OVERLOADING
  bool operator==(const gbtBehavProfileRep<T> &) const;

  // INITIALIZATION, VALIDATION
  void SetCentroid(void);

  // GENERAL DATA ACCESS
  const T &GetRealizProb(const gbtGameNode &node) const;
  const T &GetBeliefProb(const gbtGameNode &node) const;
  gbtVector<T> GetNodeValue(const gbtGameNode &node) const;
  T GetInfosetProb(const gbtGameInfoset &iset) const;
  const T &GetInfosetValue(const gbtGameInfoset &iset) const;
  T GetActionProb(const gbtGameAction &act) const;
  const T &GetActionValue(const gbtGameAction &act) const;
  const T &GetRegret(const gbtGameAction &act) const;

  // COMPUTATION OF INTERESTING QUANTITIES

  T GetPayoff(const gbtGamePlayer &p_player) const;
  T GetLiapValue(bool p_penalty = true) const;

  T DiffActionValue(const gbtGameAction &action, 
		    const gbtGameAction &oppAction) const;
  T DiffRealizProb(const gbtGameNode &node,
		   const gbtGameAction &oppAction) const;
  T DiffNodeValue(const gbtGameNode &node, const gbtGamePlayer &player,
		  const gbtGameAction &oppAction) const;

  const T &operator()(const gbtGameAction &) const;
  T &operator()(const gbtGameAction &);

  // IMPLEMENTATION OF gbtDPVector OPERATIONS
  // These are reimplemented here to correctly handle invalidation
  // of cached information.
  const T &operator()(int a, int b, int c) const
    { return m_profile(a, b, c); }
  T &operator()(int a, int b, int c) 
    { m_cachedData = false;  return m_profile(a, b, c); }
  const T &operator[](int a) const
    { return m_profile[a]; }
  T &operator[](int a)
    { m_cachedData = false;  return m_profile[a]; }

  void operator=(const T &x)  
    { m_cachedData = false;  m_profile = x; }

  bool operator==(const gbtDPVector<T> &x) const
  { return (m_profile == x); }
  bool operator!=(const gbtDPVector<T> &x) const
  { return (m_profile != x); }

  // IMPLEMENTATION OF gbtGameObject INTERFACE
  gbtText GetLabel(void) const { return ""; }
  void SetLabel(const gbtText &) { }

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return m_support->IsTree(); }
  bool IsMatrix(void) const { return m_support->IsMatrix(); }

  gbtText GetComment(void) const { return ""; }

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const { return m_support->NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const { return m_support->GetPlayer(index); }

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const { return m_support->NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_support->GetOutcome(index); }

  bool IsConstSum(void) const { return m_support->IsConstSum(); }
  gbtNumber GetMaxPayoff(void) const { return m_support->GetMaxPayoff(); }
  gbtNumber GetMinPayoff(void) const { return m_support->GetMinPayoff(); }

  // IMPLEMENTATION OF gbtConstEfgRep INTERFACE

  // DATA ACCESS -- GENERAL
  bool IsPerfectRecall(void) const { return m_support->IsPerfectRecall(); }

  // DATA ACCESS -- PLAYERS
  gbtGamePlayer GetChance(void) const { return m_support->GetChance(); }

  // DATA ACCESS -- NODES
  int NumNodes(void) const { return m_support->NumNodes(); }
  gbtGameNode GetRoot(void) const { return m_support->GetRoot(); }
  gbtList<gbtGameNode> GetNodes(void) const
  { return m_support->GetNodes(); }
  gbtList<gbtGameNode> GetTerminalNodes(void) const
  { return m_support->GetTerminalNodes(); }

  // DATA ACCESS -- ACTIONS
  gbtPVector<int> NumActions(void) const { return m_support->NumActions(); }
  int BehavProfileLength(void) const { return m_support->BehavProfileLength(); }

  // DATA ACCESS -- INFORMATION SETS
  int TotalNumInfosets(void) const { return m_support->TotalNumInfosets(); }
  gbtArray<int> NumInfosets(void) const { return m_support->NumInfosets(); }
  int NumPlayerInfosets(void) const { return m_support->NumPlayerInfosets(); }
  int NumPlayerActions(void) const { return m_support->NumPlayerActions(); }
  gbtPVector<int> NumMembers(void) const { return m_support->NumMembers(); }

  // DATA ACCESS -- SUPPORTS
  gbtEfgSupport NewEfgSupport(void) const { return m_support->NewEfgSupport(); }

  operator gbtMixedProfile<T>(void) const;

  // DATA ACCESS -- PROFILES
  gbtBehavProfile<double> NewBehavProfile(double) const;
  gbtBehavProfile<gbtRational> NewBehavProfile(const gbtRational &) const;
  gbtBehavProfile<gbtNumber> NewBehavProfile(const gbtNumber &) const;
};

#endif  // BEHAVTREE_H
