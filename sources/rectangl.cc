//
// FILE: interval.cc -- Instantiation of gInterval class
// 
// @(#)rectangl.cc	1.1 01/07/98 
//

#include <assert.h>
#include "glist.imp"
#include "double.h"
#include "rectangl.imp"

template class gRectangle<gRational>;
template class gList< gRectangle<gRational> >;
template class gNode< gRectangle<gRational> >;
template gOutput& operator << (gOutput& output, const gRectangle<gRational>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<gRational>&);

template class gRectangle<int>;
template class gList< gRectangle<int> >;
template class gNode< gRectangle<int> >;
template gOutput& operator << (gOutput& output, const gRectangle<int>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<int>&);

template class gRectangle<double>;
template class gList< gRectangle<double> >;
template class gNode< gRectangle<double> >;
template gOutput& operator << (gOutput& output, const gRectangle<double>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<double>&);

template class gRectangle<gDouble>;
template class gList< gRectangle<gDouble> >;
template class gNode< gRectangle<gDouble> >;
template gOutput& operator << (gOutput& output, const gRectangle<gDouble>& x);
template gRectangle<gDouble> TogDouble(const gRectangle<gDouble>&);
