//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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

using namespace Gambit;

extern int g_numDecimals;

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
SolveLP(const Matrix<T> &A, const Vector<T> &b, const Vector<T> &c,
	int nequals,
	Array<T> &p_primal, Array<T> &p_dual)
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

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}


//
// Convert the LP solution represented by the vectors
// (p_primal, p_dual) back to a strategy profile.
// The primal vector is the mixed strategy for player 1, and the
// dual the mixed strategy for player 2.
//

template <class T>
void PrintSolution(const StrategySupport &p_support,
		   const Array<T> &p_primal,
		   const Array<T> &p_dual)
{
  int n1 = p_support.NumStrategies(1);
  int n2 = p_support.NumStrategies(2);

  MixedStrategyProfile<T> profile(p_support.NewMixedStrategyProfile<T>());

  for (int j = 1; j <= n1; j++) {
    profile[p_support.GetStrategy(1, j)] = p_primal[j];
  }

  for (int j = 1; j <= n2; j++) {
    profile[p_support.GetStrategy(2, j)] = p_dual[j];
  }

  PrintProfile(std::cout, "NE", profile);
}


//
// Compute and print one equilibrium by solving a linear program based
// on the strategic game representation.
//
template <class T>
void SolveStrategic(const Game &p_game)
{
  StrategySupport support(p_game);

  int m = support.NumStrategies(1);
  int k = support.NumStrategies(2);

  Matrix<T> A(1,k+1,1,m+1);
  Vector<T> b(1,k+1);
  Vector<T> c(1,m+1);
  PureStrategyProfile profile = support.GetGame()->NewPureStrategyProfile();

  T minpay = p_game->GetMinPayoff() - Rational(1);

  for (int i = 1; i <= k; i++)  {
    profile->SetStrategy(support.GetStrategy(2, i));
    for (int j = 1; j <= m; j++)  {
      profile->SetStrategy(support.GetStrategy(1, j));
      A(i, j) = Rational(minpay) - profile->GetPayoff(1);
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
  if (SolveLP(A, b, c, 1, primal, dual)) {
    PrintSolution(support, primal, dual);
  }
}

template void SolveStrategic<double>(const Game &);
template void SolveStrategic<Rational>(const Game &);
