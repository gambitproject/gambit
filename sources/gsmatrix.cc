//#
//# FILE: gsmatrix.cc -- Instantiation of common squarematrix types
//#
//# @(#)gsmatrix.cc	1.1 7/19/95
//#

#include "gsmatrix.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gSquareMatrix<float>;
TEMPLATE class gSquareMatrix<double>;
TEMPLATE class gSquareMatrix<gRational>;

TEMPLATE gOutput & operator<< (gOutput&, const gSquareMatrix<float>&);
TEMPLATE gOutput & operator<< (gOutput&, const gSquareMatrix<double>&);
TEMPLATE gOutput & operator<< (gOutput&, const gSquareMatrix<gRational>&);

