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

#ifndef LIBGAMBIT_BEHAV_H
#define LIBGAMBIT_BEHAV_H

#include "game.h"

template <class T> class gbtPVector;
template <class T> class gbtRectArray;

namespace Gambit {

///
/// MixedBehavProfile<T> implements a randomized behavior profile on
/// an extensive game.
///
template <class T> class MixedBehavProfile : private gbtDPVector<T>  {
protected:
  Game m_efg;
  BehavSupport m_support;
  mutable bool m_cached_data;

  // structures for storing cached data: nodes
  mutable gbtVector<T> m_realizProbs, m_beliefs, m_nvals, m_bvals;
  mutable Matrix<T> m_nodeValues;

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

  const T &RealizProb(const GameNode &node) const;
  T &RealizProb(const GameNode &node);

  const T &BeliefProb(const GameNode &node) const;
  T &BeliefProb(const GameNode &node);
  
  gbtVector<T> NodeValues(const GameNode &node) const
    { return m_nodeValues.Row(node->number); }
  const T &NodeValue(const GameNode &node, int pl) const
    { return m_nodeValues(node->number, pl); }
  T &NodeValue(const GameNode &node, int pl)
    { return m_nodeValues(node->number, pl); }

  T IsetProb(const GameInfoset &iset) const;

  const T &IsetValue(const GameInfoset &iset) const;
  T &IsetValue(const GameInfoset &iset);

  const T &ActionValue(const GameAction &act) const 
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetNumber(),
			    act->GetInfoset()->GetNumber(),
			    act->m_number); }
  T &ActionValue(const GameAction &act)
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetNumber(),
			    act->GetInfoset()->GetNumber(),
			    act->m_number); }
  
  T ActionProb(const GameAction &act) const;

  const T &Regret(const GameAction &act) const;
  T &Regret(const GameAction &);

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES

  void Payoff(GameNodeRep *, T, int, T &) const;
  
  void ComputeSolutionDataPass2(const GameNode &node) const;
  void ComputeSolutionDataPass1(const GameNode &node) const;
  void ComputeSolutionData(void) const;

  void BehaviorStrat(const Game &, int, GameNode &);
  void RealizationProbs(const MixedStrategyProfile<T> &, const Game &,
			int pl, const gbtArray<int> &, GameNode);

public:
  /// @name Lifecycle
  //@{
  MixedBehavProfile(const BehavSupport &);
  MixedBehavProfile(const MixedBehavProfile<T> &);
  MixedBehavProfile(const MixedStrategyProfile<T> &);
  virtual ~MixedBehavProfile();

  MixedBehavProfile<T> &operator=(const MixedBehavProfile<T> &);
  MixedBehavProfile<T> &operator=(const gbtVector<T> &p)
    { Invalidate(); gbtVector<T>::operator=(p); return *this;}

  //@}
  
  /// @name Operator overloading
  //@{
  bool operator==(const MixedBehavProfile<T> &) const;
  bool operator!=(const MixedBehavProfile<T> &x) const 
  { return !(*this == x); }
  //@}

  /// @name Initialization, validation
  //@{
  inline void Invalidate(void) const {m_cached_data=false;}
  void Centroid(void) const;
  //@}

  /// @name General data access
  //@{
  Game GetGame(void) const { return m_efg; }
  const BehavSupport &GetSupport(void) const { return m_support; }
  
  const T &GetRealizProb(const GameNode &node) const;
  const T &GetBeliefProb(const GameNode &node) const;
  gbtVector<T> GetNodeValue(const GameNode &node) const;
  T GetIsetProb(const GameInfoset &iset) const;
  const T &GetIsetValue(const GameInfoset &iset) const;
  T GetActionProb(const GameAction &act) const;
  const T &GetActionValue(const GameAction &act) const;
  const T &GetRegret(const GameAction &act) const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetPayoff(int p_player) const;
  gbtDPVector<T> GetBeliefs(void);
  T GetLiapValue(void) const;
  T GetLiapValueOnDefined(void) const;
  //T MaxRegret(void);

  bool IsDefinedAt(GameInfoset p_infoset) const;

  T DiffActionValue(const GameAction &action, 
		    const GameAction &oppAction) const;
  T DiffRealizProb(const GameNode &node, 
		   const GameAction &oppAction) const;
  T DiffNodeValue(const GameNode &node, const GamePlayer &player,
		  const GameAction &oppAction) const;

  //@}

  /// @name Implementation of gDPVector opreations
  // (These are reimplemented here to correctly handle invalidation
  // of cached information.)
  //@{
  const T &operator()(int a, int b, int c) const
    { return gbtDPVector<T>::operator()(a, b, c); }
  T &operator()(int a, int b, int c) 
    { Invalidate();  return gbtDPVector<T>::operator()(a, b, c); }
  const T &operator[](int a) const
    { return gbtArray<T>::operator[](a); }
  T &operator[](int a)
    { Invalidate();  return gbtArray<T>::operator[](a); }

  MixedBehavProfile<T> &operator=(const T &x)  
    { Invalidate();  gbtDPVector<T>::operator=(x);  return *this; }

  bool operator==(const gbtDPVector<T> &x) const
    { return gbtDPVector<T>::operator==(x); }
  bool operator!=(const gbtDPVector<T> &x) const
    { return gbtDPVector<T>::operator!=(x); }

  MixedBehavProfile<T> &operator+=(const MixedBehavProfile<T> &x)
    { Invalidate();  gbtDPVector<T>::operator+=(x);  return *this; }
  MixedBehavProfile<T> &operator+=(const gbtDPVector<T> &x)
    { Invalidate();  gbtDPVector<T>::operator+=(x);  return *this; }
  MixedBehavProfile<T> &operator-=(const MixedBehavProfile<T> &x)
    { Invalidate();  gbtDPVector<T>::operator-=(x);  return *this; }
  MixedBehavProfile<T> &operator*=(const T &x)
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
  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_BEHAV_H
