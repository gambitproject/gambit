//#
//# FILE: gdpvect.cc -- Instantiation of doubly partitioned vector types
//#
//# $Id$
//#

#include "gdpvect.imp"
#include "rational.h"


#ifdef __GNUG__
// explicitly instantiate classes and non-member functions
template class gDPVector<double>;
template class gDPVector<gRational>;

template gOutput & operator<< (gOutput&, const gDPVector<double>&);
template gOutput & operator<< (gOutput&, const gDPVector<gRational>&);
#elif defined(__BORLANDC__)
// whatever
#error Explicit instantiation needs to be done for Borland C++
#else
#error Unsupported compiler type.
#endif
