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
#include "gnumber.h"

//TEMPLATE class gArray<gDouble>;
//TEMPLATE class gBlock<gDouble>;

template class gMono<int>;
template gOutput& operator << (gOutput& output, const gMono<int>& x);

template class gMono<double>;
template gOutput& operator << (gOutput& output, const gMono<double>& x);

template class gMono<gRational>;
template gOutput& operator << (gOutput& output, const gMono<gRational>& x);

template class gMono<gNumber>;
template gOutput& operator << (gOutput& output, const gMono<gNumber>& x);

#ifdef GDOUBLE
template class gMono<gDouble>;
template gOutput& operator << (gOutput& output, const gMono<gDouble>& x);
#endif   // GDOUBLE

template class gList< gMono<int> >;
template class gList< gMono<double> >;
template class gList< gMono<gRational> >;
template class gList< gMono<gNumber> >;

#ifdef GDOUBLE
template class gList< gMono<gDouble> >;
#endif   // GDOUBLE
