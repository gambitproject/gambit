//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit.h"
#include "solvers/linalg/lpsolve.h"
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
// Compute and print one equilibrium by solving a linear program based
// on the strategic game representation.
//
template <class T> List<MixedStrategyProfile<T> > 
NashLpStrategySolver<T>::Solve(const Game &p_game) const
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsConstSum()) {
    throw UndefinedException("Method only valid for constant-sum games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  int m = p_game->GetPlayer(1)->GetStrategies().size();
  int k = p_game->GetPlayer(2)->GetStrategies().size();

  Matrix<T> A(1,k+1,1,m+1);
  Vector<T> b(1,k+1);
  Vector<T> c(1,m+1);
  PureStrategyProfile profile = p_game->NewPureStrategyProfile();

  Rational minpay = p_game->GetMinPayoff() - Rational(1);

  for (int i = 1; i <= k; i++)  {
    profile->SetStrategy(p_game->GetPlayer(2)->GetStrategies()[i]);
    for (int j = 1; j <= m; j++)  {
      profile->SetStrategy(p_game->GetPlayer(1)->GetStrategies()[j]);
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

  MixedStrategyProfile<T> eqm(p_game->NewMixedStrategyProfile(static_cast<T>(0)));
  for (int j = 1; j <= m; j++) {
    eqm[p_game->GetPlayer(1)->GetStrategies()[j]] = primal[j];
  }
  for (int j = 1; j <= k; j++) {
    eqm[p_game->GetPlayer(2)->GetStrategies()[j]] = dual[j];
  }
  this->m_onEquilibrium->Render(eqm);
  List<MixedStrategyProfile<T> > solution;
  solution.push_back(eqm);
  return solution;
}

template class NashLpStrategySolver<double>;
template class NashLpStrategySolver<Rational>;
