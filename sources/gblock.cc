//
// FILE: gblock.cc -- Provide implementations for commonly-used blocks
//
// $Id$
//

#include "gmisc.h"
#include "gblock.imp"
#include "gtext.h"
#include "rational.h"
#include "gnumber.h"

template class gBlock<bool>;
template class gBlock<int>;
template class gBlock<long>;
template class gBlock<float>;
template class gBlock<double>;
template class gBlock<gRational>;
template class gBlock<gText>;
template class gBlock<gNumber>;

#include "garray.imp"
template class gArray<gBlock<int> >;
template class gArray<gBlock<double> >;
template class gBlock<gArray<int> *>;
template class gBlock<gBlock<int> >;
template class gBlock<gBlock<double> >;

template gOutput &operator<<(gOutput &, const gBlock<bool> &);
template gOutput &operator<<(gOutput &, const gBlock<int> &);
template gOutput &operator<<(gOutput &, const gBlock<long> &);
template gOutput &operator<<(gOutput &, const gBlock<float> &);
template gOutput &operator<<(gOutput &, const gBlock<double> &);
template gOutput &operator<<(gOutput &, const gBlock<gRational> &);
template gOutput &operator<<(gOutput &, const gBlock<gText> &);
template gOutput &operator<<(gOutput &, const gArray<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gArray<gBlock<double> > &);
template gOutput &operator<<(gOutput &, const gBlock<gArray<int> *> &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<double> > &);


