//#
//# FILE: gblock.cc -- Provide implementations for commonly-used blocks
//#
//# $Id$
//#

#include "basic.h"
#include "gblock.imp"
#include "gstring.h"
#include "gnumber.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gBlock<bool>;
TEMPLATE class gBlock<int>;
TEMPLATE class gBlock<int *>;
TEMPLATE class gBlock<long>;
TEMPLATE class gBlock<double>;
TEMPLATE class gBlock<double *>;
TEMPLATE class gBlock<gInteger>;
TEMPLATE class gBlock<gRational>;
TEMPLATE class gBlock<gRational *>;
TEMPLATE class gBlock<gNumber>;
TEMPLATE class gBlock<gString>;
TEMPLATE class gBlock<gBlock<int> >;
TEMPLATE class gBlock<gBlock<double> >;


TEMPLATE gOutput &operator<<(gOutput &, const gBlock<bool> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<int> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<int *> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<long> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<double *> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gInteger> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gRational> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gRational *> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gNumber> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gString> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gBlock<int> > &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gBlock<double> > &);
