//#
//# FILE: gdpvect.cc -- Instantiation of doubly partitioned vector types
//#
//# $Id$
//#

#include "gdpvect.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gDPVector<double>;
TEMPLATE class gDPVector<gRational>;

TEMPLATE gOutput & operator<< (gOutput&, const gDPVector<double>&);
TEMPLATE gOutput & operator<< (gOutput&, const gDPVector<gRational>&);
