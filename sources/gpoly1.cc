
//#
//# FILE: gpoly1.cc -- Instantiation of gPoly class
//# @(#) gpoly1.cc	1.5 9/7/95
//#

#include "glist.imp"
#include "garray.imp"
#include "gblock.imp"
#include "gpoly1.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gArray< gPoly1< int > * >;
TEMPLATE class gArray< gPoly1< double > * >;
TEMPLATE class gArray< gPoly1< gRational > *>;
TEMPLATE class gArray< Variable * >;

TEMPLATE class gBlock<Variable *>;

TEMPLATE class gList< gPoly1<int> * >;
TEMPLATE class gNode< gPoly1<int> * >;
TEMPLATE class gList< gPoly1<gRational> * >;
TEMPLATE class gNode< gPoly1<gRational> * >;
TEMPLATE class gList< gPoly1<double> * >;
TEMPLATE class gNode< gPoly1<double> * >;
TEMPLATE class gList< gPoly1<long> * >;
TEMPLATE class gNode< gPoly1<long> * >;

TEMPLATE class gPoly1<int>;
TEMPLATE gPoly1<int> operator*(const int val, const gPoly1<int> poly);
TEMPLATE gPoly1<int> operator*(const gPoly1<int> poly, const int val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly1<int> &y);

TEMPLATE class gPoly1<gRational>;
TEMPLATE gPoly1<gRational> operator*(const gRational val, const gPoly1<gRational> poly);
TEMPLATE gPoly1<gRational> operator*(const gPoly1<gRational> poly, const gRational val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly1<gRational> &y);

TEMPLATE class gPoly1<double>;
TEMPLATE gPoly1<double> operator*(const double val, const gPoly1<double> poly);
TEMPLATE gPoly1<double> operator*(const gPoly1<double> poly, const double val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly1<double> &y);

/*
TEMPLATE class gPoly1<long>;
TEMPLATE gPoly1<long> operator*(const long val, const gPoly1<long> poly);
TEMPLATE gPoly1<long> operator*(const gPoly1<long> poly, const long val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly1<long> &y);
*/



