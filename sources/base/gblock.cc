//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION;
// Provide instantiations for commonly-used blocks
//


#include "base.h"
#include "garray.imp"
#include "gblock.imp"

template class gBlock<bool>;
template class gBlock<int>;
template class gBlock<long>;
template class gBlock<float>;
template class gBlock<double>;
template class gBlock<gText>;

template class gArray<gBlock<int> >;
template class gArray<gBlock<double> >;
template class gBlock<gArray<int> *>;
template class gBlock<gBlock<int> >;
template class gBlock<gBlock<double> >;
template class gArray<gArray<gText> >;
template class gBlock<gArray<gText> >;

#ifndef __BCC55__
template bool operator==(const gArray<gText> &, const gArray<gText> &);
template bool operator!=(const gArray<gText> &, const gArray<gText> &);
#endif  // __BCC55__

template gOutput &operator<<(gOutput &, const gBlock<bool> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<int> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<long> &);
template gOutput &operator<<(gOutput &, const gBlock<float> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<double> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<gText> &);
template gOutput &operator<<(gOutput &, const gArray<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gArray<gBlock<double> > &);
template gOutput &operator<<(gOutput &, const gBlock<gArray<int> *> &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<double> > &);


