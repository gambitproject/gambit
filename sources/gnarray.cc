//#
//# FILE: gnarray.cc -- Provide implementations of commonly used gNArray<T>
//#
//# $Id$
//#

#include "gnarray.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gNArray<double>;
TEMPLATE class gNArray<gRational>;






