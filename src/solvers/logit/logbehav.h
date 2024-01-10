//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/logbehav.h
// Behavior strategy profile where action probabilities are represented using
// logarithms.
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

#ifndef LOGBEHAV_H
#define LOGBEHAV_H

using namespace Gambit;

///
/// This is a modified behavior profile class used in the tracing procedure.
/// It assumes that all probabilities are positive; therefore, the log
/// of the probability is well-defined.  Because of this, the realization
/// probability for any node can be computed by taking the sum of the log
/// probabilities of the actions leading up to it, and then exponentiating
/// the sum.
///
/// This means that beliefs can be computed accurately even for information
/// sets for which the information set realization probability is going
/// to zero.  In computing beliefs, for each information set, we find the
/// member reached with maximum probability.  Observe that it must be that
/// this node's realization probability divided by the information set's
/// realization probability must be bounded away from zero.  Beliefs are
/// then computed by subtracting the other nodes' log-realization probability
/// from this "leading" node's log-realization probability, and then
/// exponentiating the result.
///
/// This procedure is significant for this application because it is
/// necessary to accurately compute beliefs for information sets whose
/// realization probabilities are going to zero, so as to be able to
/// get a good approximation to the limiting sequential equilibrium.
///
template <class T> class LogBehavProfile : private DVector<T>  {
protected:
  BehaviorSupportProfile m_support;
  DVector<T> m_logProbs;

  mutable bool m_cacheValid;

  // structures for storing cached data: nodes
  mutable Vector<T> m_realizProbs, m_logRealizProbs;
  mutable Vector<T> m_beliefs;
  mutable Matrix<T> m_nodeValues;

  // structures for storing cached data: information sets
  mutable PVector<T> m_infosetValues;

  // structures for storing cached data: actions
  mutable DVector<T> m_actionValues;   // aka conditional payoffs

  const T &ActionValue(const GameAction &act) const 
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

public:
  /// @name Lifecycle
  //@{
  explicit LogBehavProfile(const BehaviorSupportProfile &);
  ~LogBehavProfile() override = default;

  LogBehavProfile<T> &operator=(const LogBehavProfile<T> &) = delete;
  LogBehavProfile<T> &operator=(const Vector<T> &p)
    { Invalidate(); Vector<T>::operator=(p); return *this;}
  LogBehavProfile<T> &operator=(const T &x)  
    { Invalidate(); DVector<T>::operator=(x); return *this; }

  //@}
  
  /// @name Operator overloading
  //@{
  bool operator==(const LogBehavProfile<T> &) const;
  bool operator!=(const LogBehavProfile<T> &x) const 
  { return !(*this == x); }

  const T &GetProb(const GameAction &p_action) const
    { return (*this)(p_action->GetInfoset()->GetPlayer()->GetNumber(),
		     p_action->GetInfoset()->GetNumber(),
		     m_support.GetIndex(p_action)); }
  const T &GetLogProb(int a, int b, int c) const
    { return m_logProbs(a, b, c); }
  const T &GetProb(int a, int b, int c) const
    { return DVector<T>::operator()(a, b, c); }

  void SetLogProb(int a, const T &p_value)
    { Invalidate();  
      m_logProbs[a] = p_value;
      Array<T>::operator[](a) = exp(p_value);
    }
  //@}

  /// @name Initialization, validation
  //@{
  /// Force recomputation of stored quantities
  void Invalidate() const { m_cacheValid = false; }
  /// Set the profile to the centroid
  void SetCentroid();
  //@}

  /// @name General data access
  //@{
  int Length() const { return Array<T>::Length(); }
  Game GetGame() const { return m_support.GetGame(); }
  const BehaviorSupportProfile &GetSupport() const { return m_support; }
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetPayoff(int p_player) const;

  const T &GetRealizProb(const GameNode &node) const;
  const T &GetBeliefProb(const GameNode &node) const;
  Vector<T> GetPayoff(const GameNode &node) const;
  T GetInfosetProb(const GameInfoset &iset) const;
  const T &GetPayoff(const GameInfoset &iset) const;
  T GetActionProb(const GameAction &act) const;
  T GetLogActionProb(const GameAction &) const;
  const T &GetPayoff(const GameAction &act) const;

  T DiffActionValue(const GameAction &action, 
		    const GameAction &oppAction) const;
  T DiffNodeValue(const GameNode &node, const GamePlayer &player,
		  const GameAction &oppAction) const;

  //@}
};

#endif // LOGBEHAV_H
