//#
//# FILE: glpsolve.cc -- Instantiation of common LP solvers
//#
//# @(#)glpsolve.cc	1.4 5/3/95
//#

#include "glpsolve.imp"
#include "rational.h"
#include "gtableau.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gTableau<double>;
TEMPLATE class gTableau<gRational>;
TEMPLATE class gTableau<gDouble>;

TEMPLATE class gLPTableau<double>;
TEMPLATE class gLPTableau<gRational>;
TEMPLATE class gLPTableau<gDouble>;


