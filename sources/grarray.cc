//
// FILE: grarray.cc -- Instantiation of common gRectArray types
//
// $Id$
//

#include "grarray.imp"
#include "gnumber.h"
#include "gtext.h"

template class gRectArray<int>;
template class gRectArray<double>;
template class gRectArray<gInteger>;
template class gRectArray<gRational>;
template class gRectArray<gNumber>;

template class gRectArray<gText>;

template gOutput &operator<<(gOutput &, const gRectArray<int> &);
template gOutput &operator<<(gOutput &, const gRectArray<double> &);
template gOutput &operator<<(gOutput &, const gRectArray<gInteger> &);
template gOutput &operator<<(gOutput &, const gRectArray<gRational> &);
template gOutput &operator<<(gOutput &, const gRectArray<gText> &);
