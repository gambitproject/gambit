//
// FILE: monomial.cc -- Instantiation of gMono classes
//
// $Id$
//

#include "monomial.imp"
#include "glist.imp"
#include "double.h"
#include "garray.imp"
#include "gblock.imp"

//TEMPLATE class gArray<gDouble>;
//TEMPLATE class gBlock<gDouble>;

template class gMono<int>;
template gOutput& operator << (gOutput& output, const gMono<int>& x);

template class gMono<double>;
template gOutput& operator << (gOutput& output, const gMono<double>& x);

template class gMono<gRational>;
template gOutput& operator << (gOutput& output, const gMono<gRational>& x);

#ifdef GDOUBLE
template class gMono<gDouble>;
template gOutput& operator << (gOutput& output, const gMono<gDouble>& x);
#endif   // GDOUBLE

template class gList< gMono<int> >;
template class gNode< gMono<int> >;

template class gList< gMono<double> >;
template class gNode< gMono<double> >;

template class gList< gMono<gRational> >;
template class gNode< gMono<gRational> >;

#ifdef GDOUBLE
template class gList< gMono<gDouble> >;
template class gNode< gMono<gDouble> >;
#endif   // GDOUBLE
