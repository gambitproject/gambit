//#
//# FILE: glpsolv2.cc -- Instantiation of a specialized LP solvers
//#
//# @(#)glpsolve.cc	1.0 10/26/95
//#

#include "glpsolv2.imp"
#include "rational.h"

template class gLPTableau2<double>;
template class gLPTableau2<gRational>;
