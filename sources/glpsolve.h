//
// FILE: glpsolve.h -- Linear program solution module
//
// $Id$
//

#ifndef GLPSOLVE_H
#define GLPSOLVE_H

#include "gtableau.h"

//
// This class implements a LP solver.  Its constructor takes as input a
// LP problem of the form minimize Cx subject to Ax<=B, x>=0, and computes
// the optimum values of x and Cx.  These can be accessed using the
// member functions provided.
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.
//
template <class T> class gLPTableau : public gTableau<T> {
  private:
    int phase, well_formed, feasible, bounded;
    T cost;
    gVector<T> optimum;

    int ComputePivot(int Udim, const gMatrix<T> &A, const gVector<T> &B,
		     const gVector<T> &C);

  public:
    gLPTableau(const gMatrix<T> &A, const gVector<T> &B, const gVector<T> &C);
    ~gLPTableau();
    
    T OptimumCost(void) const;
    const gVector<T> &OptimumVector(void) const;
    int IsFeasible(void) const;
    int IsBounded(void) const;
    int IsWellFormed(void) const;
};

#endif   // GLPSOLVE_H


