//
// FILE: interval.cc -- Instantiation of gInterval class
// 
// $Id$
//

#include <assert.h>
#include "interval.imp"
#include "base/glist.imp"
#include "math/double.h"
#include "math/gnumber.h"

template class gInterval<gRational>;
template class gList< gInterval<gRational> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gInterval<gRational>& x);
#endif  // __BCC55__

template class gInterval<int>;
template class gList< gInterval<int> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gInterval<int>& x);
#endif  // __BCC55__


template class gInterval<double>;
template class gList< gInterval<double> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gInterval<double>& x);
#endif  // __BCC55__

template class gInterval<gDouble>;
template class gList< gInterval<gDouble> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gInterval<gDouble>& x);
#endif  // __BCC55__

template class gInterval<gNumber>;
template class gList< gInterval<gNumber> >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, const gInterval<gNumber>& x);
#endif  // __BCC55__

