//
// FILE: glpsolv2.cc -- Instantiation of a specialized LP solvers
//
// $Id$
//

#include "glpsolv2.imp"
#include "rational.h"

template class gLPTableau2<double>;
template class gLPTableau2<gRational>;
