//#
//# FILE: lpsolve.h -- Linear program solution module
//#
//# $Id$
//#

#ifndef LPSOLVE_H
#define LPSOLVE_H

#include "tableau.h"
#include "bfs.h"
#include "gstatus.h"

//
// This class implements a LP solver.  Its constructor takes as input a
// LP problem of the form maximize c x subject to A x<=b, x >= 0, and computes
// the optimum values of x and c x.  These can be accessed using the
// member functions provided.
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.
//

template <class T> class LPSolve {
private:
  int  well_formed, feasible, bounded, aborted, flag, nvars, neqns;
  T total_cost,eps1,eps2,eps3,tmin;
  BFS<T> opt_bfs,dual_bfs;
  const gMatrix<T> &A;   // needed?
  const gVector<T> &b, &c;  // needed?
  gMatrix<T> *AA;
  LPTableau<T> *tab;
  gVector<int> *UB, *LB;
  gVector<T> *ub, *lb, *xx, *cost; 
  gVector<T> y, x, d;
  gStatus &status;
  
  void Solve(int phase = 0);
  int Enter(void);
  int Exit(int);
public:
  LPSolve(const gMatrix<T> &A, const gVector<T> &B, const gVector<T> &C,
	  int nequals = 0, gStatus &s = gstatus );   // nequals = number of equalies (last nequals rows)
//  LPSolve(const gMatrix<T> &A, const gVector<T> &B, 
//	  const gVector<T> &C,  const gVector<int> &sense, 
//	  const gVector<int> &LB,  const gVector<T> &lb, 
//	  const gVector<int> &UB, const gVector<T> &ub);
  ~LPSolve();
  
  T OptimumCost(void) const;
  const gVector<T> &OptimumVector(void) const;
  
  int IsAborted(void) const;
  int IsWellFormed(void) const;
  int IsFeasible(void) const;
  int IsBounded(void) const;
  long NumPivots(void) const;
  void OptBFS(BFS<T> &b) const;
};

#endif   // LPSOLVE_H








