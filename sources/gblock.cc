//#
//# FILE: gblock.cc -- Provide implementations for commonly-used blocks
//#
//# $Id$
//#

#include "basic.h"
#include "gblock.imp"
#include "gstring.h"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__

class gArray<bool>;
class gArray<int>;
class gArray<long>;
class gArray<float>;
class gArray<double>;
class gArray<gRational>;
class gArray<gString>;

class gArray<gArray<int> >;
class gArray<gArray<int> *>;
class gArray<gArray<double> >;

#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gBlock<bool>;
TEMPLATE class gBlock<int>;
TEMPLATE class gBlock<long>;
TEMPLATE class gBlock<float>;
TEMPLATE class gBlock<double>;
TEMPLATE class gBlock<gRational>;
TEMPLATE class gBlock<gString>;

#include "garray.imp"
TEMPLATE class gArray<gBlock<int> >;
TEMPLATE class gArray<gBlock<double> >;
TEMPLATE class gBlock<gArray<int> *>;
TEMPLATE class gBlock<gBlock<int> >;
TEMPLATE class gBlock<gBlock<double> >;

TEMPLATE gOutput &operator<<(gOutput &, const gBlock<bool> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<int> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<long> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<float> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gRational> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gString> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gBlock<int> > &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gBlock<double> > &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gArray<int> *> &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gBlock<int> > &);
TEMPLATE gOutput &operator<<(gOutput &, const gBlock<gBlock<double> > &);


