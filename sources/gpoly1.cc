//
// FILE: gpoly1.cc -- Instantiation of gPoly class
//
// $Id$
//

#include "glist.imp"
#include "garray.imp"
#include "gblock.imp"
#include "gpoly1.imp"
#include "rational.h"

template class gArray< gPoly1< int > * >;
template class gArray< gPoly1< double > * >;
template class gArray< gPoly1< gRational > *>;
template class gArray< Variable * >;

template class gBlock<Variable *>;

template class gList< gPoly1<int> * >;
template class gNode< gPoly1<int> * >;
template class gList< gPoly1<gRational> * >;
template class gNode< gPoly1<gRational> * >;
template class gList< gPoly1<double> * >;
template class gNode< gPoly1<double> * >;
template class gList< gPoly1<long> * >;
template class gNode< gPoly1<long> * >;

template class gPoly1<int>;
template gPoly1<int> operator*(const int val, const gPoly1<int> poly);
template gPoly1<int> operator*(const gPoly1<int> poly, const int val);
template gOutput &operator<<(gOutput &f, const gPoly1<int> &y);

template class gPoly1<gRational>;
template gPoly1<gRational> operator*(const gRational val, const gPoly1<gRational> poly);
template gPoly1<gRational> operator*(const gPoly1<gRational> poly, const gRational val);
template gOutput &operator<<(gOutput &f, const gPoly1<gRational> &y);

template class gPoly1<double>;
template gPoly1<double> operator*(const double val, const gPoly1<double> poly);
template gPoly1<double> operator*(const gPoly1<double> poly, const double val);
template gOutput &operator<<(gOutput &f, const gPoly1<double> &y);

/*
template class gPoly1<long>;
template gPoly1<long> operator*(const long val, const gPoly1<long> poly);
template gPoly1<long> operator*(const gPoly1<long> poly, const long val);
template gOutput &operator<<(gOutput &f, const gPoly1<long> &y);
*/



