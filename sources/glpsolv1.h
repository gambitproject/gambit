//
// FILE: glpsolv1.h -- Linear program solution module
//
// $Id$
//

#ifndef GLPSOLV1_H
#define GLPSOLV1_H

#include "glpsolve.h"

//
// This class implements an LP solver for the problem 
//
//      minimize Cx subject to Ax<=B, Dx=E, x>=0
//
// Its constructor takes as input the matrices A and D and the vectors
// B, C, and E, where the components of these are, potentially, members
// of any ordered field.  The optimum values of x and Cx can be accessed 
// using the member functions provided.
//
// All computation is done in the class constructor; when the constructor
// returns the computation has completed.
//
// The method of computation is simply to appeal to glpsolve.h to solve
// the equivalent problem  
//
//      minimize Cx subject to Ax<=B, Dx<=E,-Dx<=-E, x>=0
//
// The primary design consideration was to completely mimic the 
// interface of glpsolve.h.

template <class T> class gLPTableau1 : public gTableau<T> {
  private:
    int well_formed, feasible, bounded;
    T cost;
    gVector<T> optimum;

  public:
    gLPTableau1(const gMatrix<T> &A, const gVector<T> &B, const gVector<T> &C, 
		const gMatrix<T> &D, const gVector<T> &E);
    ~gLPTableau1()   { }
    
    inline T OptimumCost(void) const                     { return cost;       }
    inline const gVector<T> &OptimumVector(void) const   { return optimum;    }
    inline int IsFeasible(void) const                    { return feasible;   }
    inline int IsBounded(void) const                     { return bounded;    }
    inline int IsWellFormed(void) const                  { return well_formed;}
};

#endif   // GLPSOLVE1_H


