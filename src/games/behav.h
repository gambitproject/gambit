//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/behav.h
// Behavior strategy profile classes
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

namespace Gambit {

///
/// MixedBehaviorProfile<T> implements a randomized behavior profile on
/// an extensive game.
///
template <class T> class MixedBehaviorProfile : public DVector<T>  {
protected:
  BehaviorSupportProfile m_support;

  mutable bool m_cacheValid;

  // structures for storing cached data: nodes
  mutable Vector<T> m_realizProbs, m_beliefs, m_nvals, m_bvals;
  mutable Matrix<T> m_nodeValues;

  // structures for storing cached data: information sets
  mutable PVector<T> m_infosetValues;

  // structures for storing cached data: actions
  mutable DVector<T> m_actionValues;   // aka conditional payoffs
  mutable DVector<T> m_gripe;

  const T &ActionValue(const GameAction &act) const 
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetNumber(),
			    act->GetInfoset()->GetNumber(),
			    act->GetNumber()); }
  T &ActionValue(const GameAction &act)
    { return m_actionValues(act->GetInfoset()->GetPlayer()->GetNumber(),
			    act->GetInfoset()->GetNumber(),
			    act->GetNumber()); }
  
  /// @name Auxiliary functions for computation of interesting values
  //@{
  void GetPayoff(GameTreeNodeRep *, const T &, int, T &) const;
  
  void ComputeSolutionDataPass2(const GameNode &node) const;
  void ComputeSolutionDataPass1(const GameNode &node) const;
  void ComputeSolutionData() const;
  //@}

  /// @name Converting mixed strategies to behavior
  //@{
  void BehaviorStrat(int, GameTreeNodeRep *);
  void RealizationProbs(const MixedStrategyProfile<T> &,
			int pl, const Array<int> &, GameTreeNodeRep *);
  //@}

public:
  /// @name Lifecycle
  //@{
  explicit MixedBehaviorProfile(const Game &);
  explicit MixedBehaviorProfile(const BehaviorSupportProfile &);
  MixedBehaviorProfile(const MixedBehaviorProfile<T> &);
  explicit MixedBehaviorProfile(const MixedStrategyProfile<T> &);
  ~MixedBehaviorProfile() override = default;

  MixedBehaviorProfile<T> &operator=(const MixedBehaviorProfile<T> &);
  MixedBehaviorProfile<T> &operator=(const Vector<T> &p)
    { Invalidate(); Vector<T>::operator=(p); return *this;}
  MixedBehaviorProfile<T> &operator=(const T &x)  
    { Invalidate(); DVector<T>::operator=(x); return *this; }

  //@}
  
  /// @name Operator overloading
  //@{
  bool operator==(const MixedBehaviorProfile<T> &) const;
  bool operator!=(const MixedBehaviorProfile<T> &x) const 
  { return !(*this == x); }

  bool operator==(const DVector<T> &x) const
  { return DVector<T>::operator==(x); }
  bool operator!=(const DVector<T> &x) const
  { return DVector<T>::operator!=(x); }

  const T &operator[](const GameAction &p_action) const
    { return (*this)(p_action->GetInfoset()->GetPlayer()->GetNumber(),
		     p_action->GetInfoset()->GetNumber(),
		     m_support.GetIndex(p_action)); }
  T &operator[](const GameAction &p_action)
    { return (*this)(p_action->GetInfoset()->GetPlayer()->GetNumber(),
		     p_action->GetInfoset()->GetNumber(),
		     m_support.GetIndex(p_action)); }

  const T &operator()(const GameAction &p_action) const
    { return (*this)(p_action->GetInfoset()->GetPlayer()->GetNumber(),
		     p_action->GetInfoset()->GetNumber(),
		     m_support.GetIndex(p_action)); }
  T &operator()(const GameAction &p_action)
    { return (*this)(p_action->GetInfoset()->GetPlayer()->GetNumber(),
		     p_action->GetInfoset()->GetNumber(),
		     m_support.GetIndex(p_action)); }

  const T &operator()(int a, int b, int c) const
    { return DVector<T>::operator()(a, b, c); }
  T &operator()(int a, int b, int c) 
    { Invalidate();  return DVector<T>::operator()(a, b, c); }
  const T &operator[](int a) const
    { return Array<T>::operator[](a); }
  T &operator[](int a)
    { Invalidate();  return Array<T>::operator[](a); }

  MixedBehaviorProfile<T> &operator+=(const MixedBehaviorProfile<T> &x)
    { Invalidate();  DVector<T>::operator+=(x);  return *this; }
  MixedBehaviorProfile<T> &operator+=(const DVector<T> &x)
    { Invalidate();  DVector<T>::operator+=(x);  return *this; }
  MixedBehaviorProfile<T> &operator-=(const MixedBehaviorProfile<T> &x)
    { Invalidate();  DVector<T>::operator-=(x);  return *this; }
  MixedBehaviorProfile<T> &operator*=(const T &x)
    { Invalidate();  DVector<T>::operator*=(x);  return *this; }
  //@}

  /// @name Initialization, validation
  //@{
  /// Force recomputation of stored quantities
  void Invalidate() const { m_cacheValid = false; }
  /// Set the profile to the centroid
  void SetCentroid();
  /// Set the behavior at any undefined information set to the centroid
  void UndefinedToCentroid();
  /// Create a new behaviour strategy profile where strategies are played
  /// in the same proportions, but with probabilities for each player
  /// summing to one.
  MixedBehaviorProfile<T> Normalize() const;
  /// Generate a random behavior strategy profile according to the uniform distribution
  void Randomize();
  /// Generate a random behavior strategy profile according to the uniform distribution
  /// on a grid with spacing p_denom
  void Randomize(int p_denom);
  //@}

  /// @name General data access
  //@{
  int Length() const { return Array<T>::Length(); }
  Game GetGame() const { return m_support.GetGame(); }
  const BehaviorSupportProfile &GetSupport() const { return m_support; }
  
  bool IsDefinedAt(GameInfoset p_infoset) const;
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetPayoff(int p_player) const;
  T GetLiapValue(bool p_definedOnly = false) const;

  const T &GetRealizProb(const GameNode &node) const;
  T GetRealizProb(const GameInfoset &iset) const;
  const T &GetBeliefProb(const GameNode &node) const;
  Vector<T> GetPayoff(const GameNode &node) const;
  const T &GetPayoff(const GameInfoset &iset) const;
  const T &GetPayoff(const GameAction &act) const;
  T GetActionProb(const GameAction &act) const;
  const T &GetRegret(const GameAction &act) const;

  T DiffActionValue(const GameAction &action, 
		    const GameAction &oppAction) const;
  T DiffRealizProb(const GameNode &node, 
		   const GameAction &oppAction) const;
  T DiffNodeValue(const GameNode &node, const GamePlayer &player,
		  const GameAction &oppAction) const;

  MixedStrategyProfile<T> ToMixedProfile() const;

  //@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_BEHAV_H
