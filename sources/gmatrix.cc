//#
//# FILE: gmatrix.cc -- Instantiation of common matrix types
//#
//# $Id$
//#

#include "gmatrix.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gMatrix<double>;
TEMPLATE class gMatrix<gRational>;
TEMPLATE class gMatrix<int>;

TEMPLATE gVector<double> operator* (const gVector<double>&,
				    const gMatrix<double>&);
TEMPLATE gVector<gRational> operator* (const gVector<gRational>&,
				    const gMatrix<gRational>&);
TEMPLATE gVector<int> operator* (const gVector<int>&,
				    const gMatrix<int>&);


TEMPLATE gOutput & operator<< (gOutput&, const gMatrix<double>&);
TEMPLATE gOutput & operator<< (gOutput&, const gMatrix<gRational>&);
TEMPLATE gOutput & operator<< (gOutput&, const gMatrix<int>&);
