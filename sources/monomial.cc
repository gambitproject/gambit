//
// FILE: monomial.cc -- Instantiation of gMono classes
//
// @(#)monomial.cc	1.2 15 Aug 1996
//

#include "monomial.imp"
#include "glist.imp"
#include "double.h"
#include "garray.imp"
#include "gblock.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gArray<gDouble>;
TEMPLATE class gBlock<gDouble>;

TEMPLATE class gMono<int>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<int>& x);

TEMPLATE class gMono<double>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<double>& x);

TEMPLATE class gMono<gRational>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<gRational>& x);

TEMPLATE class gMono<gDouble>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<gDouble>& x);

TEMPLATE class gList< gMono<int> >;
TEMPLATE class gNode< gMono<int> >;

TEMPLATE class gList< gMono<double> >;
TEMPLATE class gNode< gMono<double> >;

TEMPLATE class gList< gMono<gRational> >;
TEMPLATE class gNode< gMono<gRational> >;

TEMPLATE class gList< gMono<gDouble> >;
TEMPLATE class gNode< gMono<gDouble> >;
