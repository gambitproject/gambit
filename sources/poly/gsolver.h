//
// File: gsolver.h  --  Declaration of gSolver data type
// 
// $Id$ 
//

#ifndef GSOLVER_H
#define GSOLVER_H

#include "ideal.h"
#include "math/double.h"
#include "numerical/linrcomb.h"
#include "gpolylst.h"

template <class T> class gSolver {

private:
  const gPolyList<T>& InputList;
  const gIdeal<T>     TheIdeal;

// Conversion
  gList<gPoly<gDouble> > BasisTogDouble() const;

// Recursive Call in Solver
  gList<gVector<gDouble> >  
    ContinuationSolutions(const gList<gPoly<gDouble> >& list,
			  const int dmnsn,
			  const int curvar,
			  const gVector<gDouble>& knownvals);

public:

// Constructor and Destructor
  gSolver(const term_order* Order,const gPolyList<T>& Inputs);
  gSolver(const gSolver<T>&);
  ~gSolver();

  bool                     IsZeroDimensional();
  gList<gVector<gDouble> > Roots();
};

#endif // GSOLVER_H
