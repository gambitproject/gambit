//#
//# FILE: lpsolve.cc -- Instantiation of common LP solvers
//#
//# $Id$
//#

#include "lpsolve.imp"
#include "rational.h"


template class LPSolve<double>;
template class LPSolve<gRational>;

