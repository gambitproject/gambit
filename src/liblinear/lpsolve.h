//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/lpsolve.h
// Interface to LP solvers
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

#ifndef LPSOLVE_H
#define LPSOLVE_H

#include "libgambit/libgambit.h"
#include "liblinear/lptab.h"
#include "liblinear/bfs.h"

//
// This class implements a LP solver.  Its constructor takes as input a
// LP problem of the form maximize c x subject to A x<=b, x >= 0. 
// The last k equations can represent equalities (indicated by the 
// parameter "nequals").  
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.  OptimumVector() returns the 
// solution.  The components are indexed by the columns of A, with the 
// excess columns representing the artificial and slack variables.   
//

template <class T> class LPSolve {
private:
  int  well_formed, feasible, bounded, aborted, flag, nvars, neqns,nequals;
  T total_cost,eps1,eps2,eps3,tmin;
  BFS<T> opt_bfs,dual_bfs;
  LPTableau<T> tab;
  Gambit::Array<bool> *UB, *LB;
  Gambit::Array<T> *ub, *lb;
  Gambit::Vector<T> *xx, *cost; 
  Gambit::Vector<T> y, x, d;

  void Solve(int phase = 0);
  int Enter(void);
  int Exit(int);
public:
  LPSolve(const Gambit::Matrix<T> &A, const Gambit::Vector<T> &B, const Gambit::Vector<T> &C,
	  int nequals);   // nequals = number of equalities (last nequals rows)
//  LPSolve(const Gambit::Matrix<T> &A, const Gambit::Vector<T> &B, 
//	  const Gambit::Vector<T> &C,  const Gambit::Vector<int> &sense, 
//	  const Gambit::Vector<int> &LB,  const Gambit::Vector<T> &lb, 
//	  const Gambit::Vector<int> &UB, const Gambit::Vector<T> &ub);
  ~LPSolve();
  
  T OptimumCost(void) const;
  const Gambit::Vector<T> &OptimumVector(void) const;
  const Gambit::List< BFS<T> > &GetAll(void);
  const LPTableau<T> &GetTableau();
  
  int IsAborted(void) const;
  int IsWellFormed(void) const;
  int IsFeasible(void) const;
  int IsBounded(void) const;
  long NumPivots(void) const;
  void OptBFS(BFS<T> &b) const;
  T Epsilon(int i = 2) const;
};

#endif   // LPSOLVE_H








