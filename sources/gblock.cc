//#
//# FILE: gblock.cc -- Provide implementations for commonly-used blocks
//#
//# $Id$
//#

#include "gblock.imp"
#include "gstring.h"
#include "gnumber.h"

template class gBlock<bool>;
template class gBlock<int>;
template class gBlock<int *>;
template class gBlock<long>;
template class gBlock<double>;
template class gBlock<double *>;
template class gBlock<gInteger>;
template class gBlock<gRational>;
template class gBlock<gRational *>;
template class gBlock<gNumber>;
template class gBlock<gString>;
template class gBlock<gBlock<int> >;
template class gBlock<gBlock<double> >;


template gOutput &operator<<(gOutput &, const gBlock<bool> &);
template gOutput &operator<<(gOutput &, const gBlock<int> &);
template gOutput &operator<<(gOutput &, const gBlock<int *> &);
template gOutput &operator<<(gOutput &, const gBlock<long> &);
template gOutput &operator<<(gOutput &, const gBlock<double> &);
template gOutput &operator<<(gOutput &, const gBlock<double *> &);
template gOutput &operator<<(gOutput &, const gBlock<gInteger> &);
template gOutput &operator<<(gOutput &, const gBlock<gRational> &);
template gOutput &operator<<(gOutput &, const gBlock<gRational *> &);
template gOutput &operator<<(gOutput &, const gBlock<gNumber> &);
template gOutput &operator<<(gOutput &, const gBlock<gString> &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<double> > &);




