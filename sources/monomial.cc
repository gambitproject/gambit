//#
//# FILE: gMono.cc -- Instantiation of gMono classes
//# @(#) gMono.cc	1.0  2/19/96
//#

#include "monomial.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gMono<gRational>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<gRational>& x);

TEMPLATE class gMono<int>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<int>& x);

TEMPLATE class gMono<double>;
TEMPLATE gOutput& operator << (gOutput& output, const gMono<double>& x);

