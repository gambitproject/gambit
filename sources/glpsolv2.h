//#
//# FILE: glpsolv2.h -- Linear program solution module
//#
//# @(#)glpsolv2.h	1.0 12/19/94
//#

#ifndef GLPSOLV2_H
#define GLPSOLV2_H

#include "glpsolve.h"

//
// This class implements an LP solver for the problem 
//
//      minimize Cx subject to Ax<=B
//
// Its constructor takes as input the matrix A and the vectors
// B, C, where the components of these are, potentially, members
// of any ordered field.  The optimum values of x and Cx can be accessed 
// using the member functions provided.
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.
//
// The method of computation is simply to appeal to glpsolve.h to solve
// the equivalent problem  
//
//      minimize C(x+ - x-) subject to A(x+ - x-)<=B, x+ >= 0, x- >= 0
//
// This is probably not best, since the answer is indeterminate, and
// consequently may be numerically unstable.

template <class T> class gLPTableau2 : public gTableau<T> {
  private:
    int well_formed, feasible, bounded;
    T cost;
    gVector<T> optimum;

  public:
    gLPTableau2(const gMatrix<T> &A, const gVector<T> &B, const gVector<T> &C);
    ~gLPTableau2()   { }
    
    inline T OptimumCost(void) const                     { return cost;       }
    inline const gVector<T> &OptimumVector(void) const   { return optimum;    }
    inline int IsFeasible(void) const                    { return feasible;   }
    inline int IsBounded(void) const                     { return bounded;    }
    inline int IsWellFormed(void) const                  { return well_formed;}
};

#endif   // GLPSOLVE2_H
