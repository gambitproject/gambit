//
// FILE: glpsolv3.cc -- Instantiation of common LP solvers
//
// $Id$
//

#include "glpsolv3.imp"
#include "rational.h"
#include "glpsolve.imp"


template class gLPTableau3<double>;
template class gLPTableau3<gDouble>;
template class gLPTableau3<gRational>;


