//#
//# FILE: lpsolve.h -- Linear program solution module
//#
//# $Id$
//#

#ifndef LPSOLVE_H
#define LPSOLVE_H

#include "tableau.h"
#include "bfs.h"

//
// This class implements a LP solver.  Its constructor takes as input a
// LP problem of the form minimize Cx subject to Ax<=B, x>=0, and computes
// the optimum values of x and Cx.  These can be accessed using the
// member functions provided.
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.
//

template <class T> class LPSolve {
private:
  int  well_formed, feasible, bounded,flag,nvars,neqs;
  T total_cost,eps1,eps2,eps3;
  BFS<T> opt_bfs,dual_bfs;
  const gMatrix<T> &A;   // needed?
  const gVector<T> &b, &c;  // needed?
  LPTableau<T> *tab;
  gVector<int> UB, LB;
  gVector<T> ub, lb, xx,y, x, d, cost;
  
  void Solve(void);
  int Enter(void);
  int Exit(int);
public:
  LPSolve(const gMatrix<T> &A, const gVector<T> &B, const gVector<T> &C,
	  int neq = 0);   // neq = number of equalies (last neq rows)
  LPSolve(const gMatrix<T> &A, const gVector<T> &B, 
	  const gVector<T> &C,  const gVector<int> &sense, 
	  const gVector<int> &LB,  const gVector<T> &lb, 
	  const gVector<int> &UB, const gVector<T> &ub);
  ~LPSolve();
  
  T OptimumCost(void) const;
  const gVector<T> &OptimumVector(void) const;
  
  int IsFeasible(void) const;
  int IsBounded(void) const;
  int IsWellFormed(void) const;
  long NumPivots(void) const;
  void OptBFS(BFS<T> &b) const;
};

#endif   // LPSOLVE_H




