//
// FILE: grarray.cc -- Instantiation of common gRectArray types
//
// $Id$
//

#include "grarray.imp"
#include "gnumber.h"
#include "gstring.h"

template class gRectArray<int>;
template class gRectArray<double>;
template class gRectArray<gRational>;
template class gRectArray<gNumber>;

template class gRectArray<gString>;

template gOutput &operator<<(gOutput &, const gRectArray<int> &);
template gOutput &operator<<(gOutput &, const gRectArray<double> &);
template gOutput &operator<<(gOutput &, const gRectArray<gRational> &);
template gOutput &operator<<(gOutput &, const gRectArray<gNumber> &);
