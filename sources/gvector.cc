//#
//# FILE: gvector.cc -- Instantiation of vector types
//#
//# $Id$
//#

#include "gvector.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__


TEMPLATE class gVector<int>;
TEMPLATE class gVector<long>;
TEMPLATE class gVector<double>;
TEMPLATE class gVector<gInteger>;
TEMPLATE class gVector<gRational>;

TEMPLATE gOutput & operator<< (gOutput&, const gVector<int>&);
TEMPLATE gOutput & operator<< (gOutput&, const gVector<long>&);
TEMPLATE gOutput & operator<< (gOutput&, const gVector<double>&);
TEMPLATE gOutput & operator<< (gOutput&, const gVector<gInteger>&);
TEMPLATE gOutput & operator<< (gOutput&, const gVector<gRational>&);
