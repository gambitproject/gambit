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
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gRectangle<gRational>& x);
#endif  // __BCC55__
template gRectangle<gDouble> TogDouble(const gRectangle<gRational>&);

template class gRectangle<double>;
template class gList< gRectangle<double> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gRectangle<double>& x);
#endif  // __BCC55__
template gRectangle<gDouble> TogDouble(const gRectangle<double>&);

template class gRectangle<gDouble>;
template class gList< gRectangle<gDouble> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gRectangle<gDouble>& x);
#endif // __BCC55__
template gRectangle<gDouble> TogDouble(const gRectangle<gDouble>&);



