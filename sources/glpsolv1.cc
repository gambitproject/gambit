//
// FILE: glpsolv1.cc -- Instantiation of a specialized LP solvers
//
// $Id$
//

#include "glpsolv1.imp"
#include "rational.h"

template class gLPTableau1<double>;
template class gLPTableau1<gRational>;
