//
// FILE: glist.cc -- Provide implementations for commonly-used lists
//
// $Id$
//

#include "base/base.h"
#include "base/glist.imp"
#include "rational.h"
#include "gnumber.h"

template class gList<gRational>;
template class gList<gNumber>;
template class gList<int>;
template class gList<long>;
template class gList<double>;
template class gList<gText>;

template class gList<gBlock<int> >;
template class gList<gArray<int> >;
template class gList<gList<int> >;
template gOutput &operator<<(gOutput &, const gList<int> &);

template class gList<bool>;
template gOutput &operator<<(gOutput &, const gList<bool> &);
template class gList<gList<bool> >;
template gOutput &operator<<(gOutput &, const gList<gList<bool> > &);
template class gList<gList<gList<bool> > >;

