//
// FILE: glist.cc -- Provide implementations for commonly-used lists
//
// $Id$
//

#include "base/base.h"
#include "base/glist.imp"

template class gList<int>;
template class gList<long>;
template class gList<double>;
template class gList<gText>;

template class gList<gBlock<int> >;
template class gList<gArray<int> >;
template class gList<gList<int> >;
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gList<int> &);
#endif  // __BCC55__

template class gList<bool>;
template gOutput &operator<<(gOutput &, const gList<bool> &);
template class gList<gList<bool> >;
template gOutput &operator<<(gOutput &, const gList<gList<bool> > &);
template class gList<gList<gList<bool> > >;

