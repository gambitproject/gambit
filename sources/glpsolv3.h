//#
//# FILE: glpsolv3.h -- Linear program solution module
//#
//# @(#)glpsolv3.h	1.6 1/16/95
//#

#ifndef GLPSOLV3_H
#define GLPSOLV3_H

#include "gtableau.h"
#include "glpsolve.h"

//
// This class implements a LP solver.  Its constructor takes as input a
// LP problem of the form minimize Cx subject to Ax=B, x>=0, and computes
// the optimum values of x and Cx.  These can be accessed using the
// member functions provided.
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.
//
// This is implemented by breaking the problem down as Ax<=B and -Ax<=-B.
// This is probably not very good, due to numerical instability.

template <class T> class gLPTableau3 : public gTableau<T> {
  private:
    int phase, well_formed, feasible, bounded;
    T cost;
    gVector<T> optimum;

    int ComputePivot(int Udim, const gMatrix<T> &A, const gVector<T> &B,
		     const gVector<T> &C);

  public:
    gLPTableau3(const gMatrix<T> &A, const gVector<T> &B, const gVector<T> &C);
    ~gLPTableau3();
    
    T OptimumCost(void) const;
    const gVector<T> &OptimumVector(void) const;
    int IsFeasible(void) const;
    int IsBounded(void) const;
    int IsWellFormed(void) const;
};

#endif   // GLPSOLVE_H


