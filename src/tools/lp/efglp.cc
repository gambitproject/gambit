//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lp/efglp.cc
// Implementation of algorithm to solve efgs via linear programming
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

#include "gambit.h"
#include "solvers/linalg/lpsolve.h"
#include "efglp.h"

using namespace Gambit;


template <class T> class NashLpBehavSolver<T>::GameData {
public:
  int ns1, ns2, ni1, ni2;
  Rational minpay;
  PVector<int> infosetIndex, infosetOffset;
  
  explicit GameData(const BehaviorSupportProfile &);

  void BuildConstraintMatrix(const BehaviorSupportProfile &,
			     Matrix<T> &, const GameNode &, const T &,
			     int, int, int, int);
  void GetBehavior(const BehaviorSupportProfile &, MixedBehaviorProfile<T> &v,
		   const Array<T> &, const Array<T> &,
		   const GameNode &, int, int);
};

template <class T>
NashLpBehavSolver<T>::GameData::GameData(const BehaviorSupportProfile &p_support)
  : infosetIndex(p_support.GetGame()->NumInfosets()), 
    infosetOffset(p_support.GetGame()->NumInfosets())
{
  infosetIndex = 0;
  infosetOffset = 0;
  ns1 = p_support.NumSequences(1);
  ns2 = p_support.NumSequences(2);
  ni1 = p_support.GetGame()->GetPlayer(1)->NumInfosets()+1;  
  ni2 = p_support.GetGame()->GetPlayer(2)->NumInfosets()+1; 
  p_support.ReachableInfosets(p_support.GetGame()->GetRoot(), infosetIndex);
  for (int iset = 1, offset = 1, index = 1; iset < ni1; iset++) {
    if (infosetIndex(1, iset) > 0) {
      infosetOffset(1, iset) = offset;
      infosetIndex(1, iset) = index++;
      offset += p_support.NumActions(1, iset);
    }
  }
  for (int iset = 1, offset = 1, index = 1; iset < ni2; iset++) {
    if (infosetIndex(2, iset) > 0) {
      infosetOffset(2, iset) = offset;
      infosetIndex(2, iset) = index++;
      offset += p_support.NumActions(2, iset);
    }
  }
  minpay = p_support.GetGame()->GetMinPayoff();
}

//
// Recursively fills the constraint matrix A for the subtree rooted at 'n'.
//
template <class T> void
NashLpBehavSolver<T>::GameData::BuildConstraintMatrix(const BehaviorSupportProfile &p_support,
						      Matrix<T> &A, 
						      const GameNode &n, 
						      const T &prob,
						      int s1, int s2, 
						      int i1, int i2)
{
  GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    A(s1,s2) += 
      (T) (Rational(prob) * static_cast<Rational>(outcome->GetPayoff(1)) - minpay);
  }

  if (n->NumChildren() == 0) {
    // Terminal node, recursion ends
    return;
  }

  if (n->GetPlayer()->IsChance()) {
    GameInfoset infoset = n->GetInfoset();
    for (int i = 1; i <= n->NumChildren(); i++) {
      BuildConstraintMatrix(p_support, A, n->GetChild(i),
			    prob * static_cast<T>(infoset->GetActionProb(i)),
			    s1, s2, i1, i2);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    i1 = infosetIndex(1, n->GetInfoset()->GetNumber());
    int snew = infosetOffset(1, n->GetInfoset()->GetNumber());
    A(s1, ns2+i1+1) = (T) 1;
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      A(snew+i, ns2+i1+1) = (T) -1;
      BuildConstraintMatrix(p_support, A, 
			    n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
			    prob, snew+i, s2, i1, i2);
    }
  }
  else {  // Must be player 2
    i2 = infosetIndex(2, n->GetInfoset()->GetNumber());
    int snew = infosetOffset(2, n->GetInfoset()->GetNumber());
    A(ns1+i2+1, s2) = (T) -1;
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      A(ns1+i2+1, snew+i) = (T) 1;
      BuildConstraintMatrix(p_support, A, 
			    n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
			    prob, s1, snew+i, i1, i2);
    }
  }
}


//
// The routine to actually solve the LP
// This routine takes an LP of the form
//    maximize c x subject to Ax>=b and x>=0,
// except the last 'nequals' constraints in A hold with equality.
// It expects the array p_primal to be the same length as the
// number of columns in A, and the routine returns the primal solution;
// similarly, the array p_dual should have the same length as the
// number of rows in A, and the routine returns the dual solution.
//
// To implement your own custom solver for this problem, simply
// replace this function.
//
template <class T> bool
NashLpBehavSolver<T>::SolveLP(const Matrix<T> &A, 
			      const Vector<T> &b, const Vector<T> &c,
			      int nequals,
			      Array<T> &p_primal, Array<T> &p_dual) const
{
  Gambit::linalg::LPSolve<T> LP(A, b, c, nequals);
  const Gambit::linalg::BFS<T> &cbfs(LP.OptimumBFS());
  
  for (int i = 1; i <= A.NumColumns(); i++) {
    if (cbfs.count(i)) {
      p_primal[i] = cbfs[i];
    }
    else {
      p_primal[i] = (T) 0;
    }
  }

  for (int i = 1; i <= A.NumRows(); i++) {
    if (cbfs.count(-i)) {
      p_dual[i] = cbfs[-i];
    }
    else {
      p_dual[i] = (T) 0;
    }
  }
  return true;
}

//
// Recursively construct the behavior profile from the sequence form
// solution represented by 'p_primal' (containing player 2's
// sequences) and 'p_dual' (containing player 1's sequences).
//
// Any information sets not reached with positive probability have
// their action probabilities set to zero.
//
template <class T> void
NashLpBehavSolver<T>::GameData::GetBehavior(const BehaviorSupportProfile &p_support,
					    MixedBehaviorProfile<T> &v,
					    const Array<T> &p_primal, 
					    const Array<T> &p_dual,
					    const GameNode &n,
					    int s1, int s2)
{
  if (n->NumChildren() == 0) {
    return;
  }

  if (n->GetPlayer()->IsChance()) {
    for (int i = 1; i <= n->NumChildren(); i++) {
      GetBehavior(p_support, v, p_primal, p_dual,
		  n->GetChild(i), s1, s2);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 2) {
    int inf = infosetIndex(2, n->GetInfoset()->GetNumber());
    int snew = infosetOffset(2, n->GetInfoset()->GetNumber());
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      if (p_primal[s1] > (T) 0) {
	v(2,inf,i) = p_primal[snew+i] / p_primal[s1];
      } 
      else {
	v(2,inf,i) = (T) 0;
      }
      GetBehavior(p_support, v, p_primal, p_dual,
		  n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
		  snew+i, s2);
    }
  }
  else {  // Must be player 1
    int inf = infosetIndex(1, n->GetInfoset()->GetNumber());
    int snew = infosetOffset(1, n->GetInfoset()->GetNumber());
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      if (p_dual[s2] > (T) 0) {
	v(1,inf,i) = p_dual[snew+i] / p_dual[s2];
      }
      else {
	v(1,inf,i) = (T) 0;
      }
      GetBehavior(p_support, v, p_primal, p_dual,
		  n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
		  s1, snew+i);
    }
  }
}


//
// Compute and print one equilibrium by solving a linear program based
// on the sequence form representation of the game.
//
template <class T> List<MixedBehaviorProfile<T> > 
NashLpBehavSolver<T>::Solve(const BehaviorSupportProfile &p_support) const
{
  if (p_support.GetGame()->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_support.GetGame()->IsConstSum()) {
    throw UndefinedException("Method only valid for constant-sum games.");
  }
  if (!p_support.GetGame()->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  Gambit::linalg::BFS<T> cbfs;
  
  GameData data(BehaviorSupportProfile(p_support.GetGame()));

  Matrix<T> A(1, data.ns1 + data.ni2, 1, data.ns2 + data.ni1);
  Vector<T> b(1, data.ns1 + data.ni2);
  Vector<T> c(1, data.ns2 + data.ni1);

  A = (T) 0;
  b = (T) 0;
  c = (T) 0;

  data.BuildConstraintMatrix(p_support, A, p_support.GetGame()->GetRoot(), 
			     (T) 1, 1, 1, 0, 0);
  A(1, data.ns2 + 1) = (T) -1;
  A(data.ns1 + 1, 1) = (T) 1;

  b[data.ns1 + 1] = (T) 1;
  c[data.ns2 + 1] = (T) -1;

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  List<MixedBehaviorProfile<T> > solution;
  if (SolveLP(A, b, c, data.ni2, primal, dual)) {
    MixedBehaviorProfile<T> profile(p_support);
    data.GetBehavior(p_support, profile, primal, dual,
		     p_support.GetGame()->GetRoot(), 1, 1);
    profile.UndefinedToCentroid();
    this->m_onEquilibrium->Render(profile);
    solution.push_back(profile);
  }
  return solution;
}

template class NashLpBehavSolver<double>;
template class NashLpBehavSolver<Rational>;

