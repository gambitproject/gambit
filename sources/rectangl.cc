//
// FILE: interval.cc -- Instantiation of gInterval class
// 
// $Id$
//

#include <assert.h>
#include "base/glist.imp"
#include "math/double.h"
#include "rectangl.imp"

template class gRectangle<gRational>;
template class gList< gRectangle<gRational> >;
template gOutput& operator << (gOutput& output, const gRectangle<gRational>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<gRational>&);

template class gRectangle<double>;
template class gList< gRectangle<double> >;
template gOutput& operator << (gOutput& output, const gRectangle<double>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<double>&);

template class gRectangle<gDouble>;
template class gList< gRectangle<gDouble> >;
template gOutput& operator << (gOutput& output, const gRectangle<gDouble>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<gDouble>&);



