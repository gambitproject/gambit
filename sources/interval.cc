//#
//# FILE: interval.cc -- Instantiation of gInterval class
//# @(#) interval.cc	1.0 2/8/96
//#

#include <assert.h>
#include "interval.imp"
#include "glist.imp"
#include "double.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gInterval<gRational>;
TEMPLATE class gList< gInterval<gRational> >;
TEMPLATE class gNode< gInterval<gRational> >;
TEMPLATE gOutput& operator << (gOutput& output, const gInterval<gRational>& x);

TEMPLATE class gInterval<int>;
TEMPLATE class gList< gInterval<int> >;
TEMPLATE class gNode< gInterval<int> >;
TEMPLATE gOutput& operator << (gOutput& output, const gInterval<int>& x);

TEMPLATE class gInterval<double>;
TEMPLATE class gList< gInterval<double> >;
TEMPLATE class gNode< gInterval<double> >;
TEMPLATE gOutput& operator << (gOutput& output, const gInterval<double>& x);

TEMPLATE class gInterval<gDouble>;
TEMPLATE class gList< gInterval<gDouble> >;
TEMPLATE class gNode< gInterval<gDouble> >;
TEMPLATE gOutput& operator << (gOutput& output, const gInterval<gDouble>& x);
