//#
//# FILE: lpsolve.cc -- Instantiation of common LP solvers
//#
//# $Id$
//#

#include "lpsolve.imp"
#include "rational.h"
#include "tableau.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class LPSolve<double>;
TEMPLATE class LPSolve<gRational>;

