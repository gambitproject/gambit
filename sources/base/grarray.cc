//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of common gRectArray types
//

#include "base.h"
#include "grarray.imp"

template class gRectArray<int>;
template class gRectArray<double>;
template class gRectArray<long double>;
template class gRectArray<gText>;

template gOutput &operator<<(gOutput &, const gRectArray<int> &);
template gOutput &operator<<(gOutput &, const gRectArray<double> &);
template gOutput &operator<<(gOutput &, const gRectArray<gText> &);
