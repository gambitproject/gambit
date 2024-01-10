//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit.h"
#include "lptab.h"
#include "bfs.h"

namespace Gambit {

namespace linalg {

///
/// This class implements a LP solver.  Its constructor takes as input a
/// LP problem of the form maximize c x subject to A x<=b, x >= 0. 
/// The last k equations can represent equalities (indicated by the 
/// parameter "nequals").  
///
/// All computation is done in the class constructor; when the constructor
/// returns the computation has completed.  OptimumVector() returns the 
/// solution.  The components are indexed by the columns of A, with the 
/// excess columns representing the artificial and slack variables.   
///
template <class T> class LPSolve {
private:
  bool well_formed, feasible, bounded;
  int flag, nvars, neqns, nequals;
  T total_cost,eps1,eps2,eps3,tmin;
  BFS<T> opt_bfs,dual_bfs;
  LPTableau<T> tab;
  Array<bool> *UB, *LB;
  Array<T> *ub, *lb;
  Vector<T> *xx, *cost; 
  Vector<T> y, x, d;

  void Solve(int phase = 0);
  int Enter();
  int Exit(int);

  static Array<int> Artificials(const Vector<T> &);
  
public:
  LPSolve(const Matrix<T> &A, const Vector<T> &B, const Vector<T> &C,
	  int nequals);   // nequals = number of equalities (last nequals rows)
  ~LPSolve();
  
  T OptimumCost() const { return total_cost; }
  const Vector<T> &OptimumVector() const { return (*xx); }
  const List< BFS<T> > &GetAll();
  const LPTableau<T> &GetTableau() const { return tab; }
  const BFS<T> &OptimumBFS() const { return opt_bfs; }
  
  bool IsWellFormed() const { return well_formed; }
  bool IsFeasible() const { return feasible; }
  bool IsBounded() const  { return bounded; }
  long NumPivots() const { return tab.NumPivots(); }
};

}  // end namespace Gambit::linalg

}  // end namespace Gambit
 
#endif   // LPSOLVE_H








