//#
//# FILE: grarray.cc -- Instantiation of common gRectArray types
//#
//# $Id$
//#

#include "grarray.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gRectArray<int>;
TEMPLATE class gRectArray<double>;
TEMPLATE class gRectArray<gRational>;

TEMPLATE gOutput &operator<<(gOutput &, const gRectArray<int> &);
TEMPLATE gOutput &operator<<(gOutput &, const gRectArray<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const gRectArray<gRational> &);

