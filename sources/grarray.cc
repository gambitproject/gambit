//#
//# FILE: grarray.cc -- Instantiation of common gRectArray types
//#
//# $Id$
//#

#include "grarray.imp"
#include "rational.h"


template class gRectArray<int>;
template class gRectArray<double>;
template class gRectArray<gRational>;

template gOutput &operator<<(gOutput &, const gRectArray<int> &);
template gOutput &operator<<(gOutput &, const gRectArray<double> &);
template gOutput &operator<<(gOutput &, const gRectArray<gRational> &);

