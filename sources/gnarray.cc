//#
//# FILE: gnarray.cc -- Provide implementations of commonly used gNArray<T>
//#
//# $Id$
//#

#ifdef __GNUG__
#pragma implementation
#elif defined(__BORLANDC__)
#pragma option -Jgd
#else
#error Unsupported compiler type.
#endif  // __GNUG__, __BORLANDC__

#include "gnarray.imp"
#include "rational.h"

typedef gNArray<double> gNArrayDouble;
typedef gNArray<gRational> gNArrayRational;


