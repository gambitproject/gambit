//#
//# FILE: gpvector.cc -- Instantiation of partitioned vector types
//#
//# $Id$
//#

#include "gpvector.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gPVector<int>;
TEMPLATE class gPVector<double>;
TEMPLATE class gPVector<gRational>;

TEMPLATE gOutput & operator<< (gOutput&, const gPVector<int>&);
TEMPLATE gOutput & operator<< (gOutput&, const gPVector<double>&);
TEMPLATE gOutput & operator<< (gOutput&, const gPVector<gRational>&);
