//#
//# FILE: lpsolve.cc -- Instantiation of common LP solvers
//#
//# $Id$
//#

#include "lpsolve.imp"
#include "rational.h"


template class LPSolve<double>;
template class LPSolve<gRational>;

template gBlock<int> Artificials(const gVector<double> &);
template gBlock<int> Artificials(const gVector<gRational> &);


