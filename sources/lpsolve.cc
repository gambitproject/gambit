//
// FILE: lpsolve.cc -- Instantiation of common LP solvers
//
// $Id$
//

#include "lpsolve.imp"
#include "math/rational.h"


template class LPSolve<double>;
template class LPSolve<gRational>;

#ifndef __BCC55__
template gBlock<int> Artificials(const gVector<double> &);
template gBlock<int> Artificials(const gVector<gRational> &);
#endif  // __BCC55__

