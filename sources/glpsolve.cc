//#
//# FILE: glpsolve.cc -- Instantiation of common LP solvers
//#
//# @(#)glpsolve.cc	1.4 5/3/95
//#

#include "glpsolve.imp"
#include "rational.h"
#include "gtableau.imp"

template class gTableau<double>;
template class gTableau<gRational>;
template class gTableau<gDouble>;

template class gLPTableau<double>;
template class gLPTableau<gRational>;
template class gLPTableau<gDouble>;


