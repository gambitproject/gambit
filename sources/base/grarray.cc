//
// FILE: grarray.cc -- Instantiation of common gRectArray types
//
// $Id$
//

#include "base/base.h"
#include "base/grarray.imp"

template class gRectArray<int>;
template class gRectArray<double>;
template class gRectArray<gText>;

template gOutput &operator<<(gOutput &, const gRectArray<int> &);
template gOutput &operator<<(gOutput &, const gRectArray<double> &);
template gOutput &operator<<(gOutput &, const gRectArray<gText> &);
