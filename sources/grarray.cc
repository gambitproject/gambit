//#
//# FILE: grarray.cc -- Instantiation of common gRectArray types
//#
//# @(#)grarray.cc	2.2 6/22/97
//#

#include "grarray.imp"
#include "rational.h"
#include "double.h"

template class gRectArray<int>;
template class gRectArray<double>;
template class gRectArray<gRational>;
template class gRectArray<gDouble>;

template gOutput &operator<<(gOutput &, const gRectArray<int> &);
template gOutput &operator<<(gOutput &, const gRectArray<double> &);
template gOutput &operator<<(gOutput &, const gRectArray<gRational> &);
template gOutput &operator<<(gOutput &, const gRectArray<gDouble> &);

