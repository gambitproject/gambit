//#
//# FILE: gsmatrix.cc -- Instantiation of common squarematrix types
//#
//# $Id$
//#

#include "gsmatrix.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gSquareMatrix<double>;
TEMPLATE class gSquareMatrix<gRational>;

