//
// $Source$
// $Date$
// $Revision$
// 
// DESCRIPTION:
// Instantiation of math container types
//

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"
#include "base/grarray.imp"
#include "base/grblock.imp"
#include "gnumber.h"
#include "complex.h"

template class gArray<gInteger>;
template class gArray<gRational>;
template class gArray<gComplex>;
template class gArray<gNumber>;
template gOutput &operator<<(gOutput &, const gArray<gInteger> &);
template gOutput &operator<<(gOutput &, const gArray<gRational> &);
template gOutput &operator<<(gOutput &, const gArray<gText> &);

template class gBlock<gRational>;
template class gBlock<gNumber>;
template gOutput &operator<<(gOutput &, const gBlock<gRational> &);
template gOutput &operator<<(gOutput &, const gBlock<gNumber> &);

template class gList<gRational>;
template class gList<gNumber>;

template class gRectArray<gInteger>;
template class gRectArray<gRational>;
template class gRectArray<gNumber>;
template gOutput &operator<<(gOutput &, const gRectArray<gInteger> &);
template gOutput &operator<<(gOutput &, const gRectArray<gRational> &);
template gOutput &operator<<(gOutput &, const gRectArray<gNumber> &);

template class gRectBlock<gNumber>;

