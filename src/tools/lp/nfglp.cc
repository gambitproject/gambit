//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lp/nfglp.cc
// Implementation of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
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

#include <unistd.h>
#include <iostream>
#include "libgambit/libgambit.h"
#include "liblinear/lpsolve.h"
#include "nfglp.h"

using namespace Gambit;

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
NashLpStrategySolver<T>::SolveLP(const Matrix<T> &A, 
				 const Vector<T> &b, const Vector<T> &c,
				 int nequals,
				 Array<T> &p_primal, Array<T> &p_dual) const
{
  LPSolve<T> LP(A, b, c, nequals);
  if (!LP.IsAborted()) {
    BFS<T> cbfs;
    LP.OptBFS(cbfs);

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
  else {
    return false;
  }
}

//
// Compute and print one equilibrium by solving a linear program based
// on the strategic game representation.
//
template <class T> List<MixedStrategyProfile<T> > 
NashLpStrategySolver<T>::Solve(const StrategySupport &p_support) const
{
  int m = p_support.NumStrategies(1);
  int k = p_support.NumStrategies(2);

  Matrix<T> A(1,k+1,1,m+1);
  Vector<T> b(1,k+1);
  Vector<T> c(1,m+1);
  PureStrategyProfile profile = p_support.GetGame()->NewPureStrategyProfile();

  Rational minpay = p_support.GetGame()->GetMinPayoff() - Rational(1);

  for (int i = 1; i <= k; i++)  {
    profile->SetStrategy(p_support.GetStrategy(2, i));
    for (int j = 1; j <= m; j++)  {
      profile->SetStrategy(p_support.GetStrategy(1, j));
      A(i, j) = minpay - profile->GetPayoff(1);
    }
    A(i,m+1) = (T) 1;
  }
  for (int j = 1; j <= m; j++) {
    A(k+1,j) = (T) 1;
  }
  A(k+1,m+1) = (T) 0;

  b = (T) 0;
  b[k+1] = (T) 1;
  c = (T) 0;
  c[m+1] = (T) 1;
  
  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  if (!SolveLP(A, b, c, 1, primal, dual)) {
    return List<MixedStrategyProfile<T> >();
  }

  int n1 = p_support.NumStrategies(1);
  int n2 = p_support.NumStrategies(2);
  MixedStrategyProfile<T> eqm(p_support.GetGame()->NewMixedStrategyProfile((T) 0));

  for (int j = 1; j <= n1; j++) {
    eqm[p_support.GetStrategy(1, j)] = primal[j];
  }
  for (int j = 1; j <= n2; j++) {
    eqm[p_support.GetStrategy(2, j)] = dual[j];
  }

  this->m_onEquilibrium->Render(eqm);
  List<MixedStrategyProfile<T> > solution;
  solution.push_back(eqm);
  return solution;
}

template class NashLpStrategySolver<double>;
template class NashLpStrategySolver<Rational>;
