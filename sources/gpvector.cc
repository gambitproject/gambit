//#
//# FILE: gpvector.cc -- Instantiation of partitioned vector types
//#
//# $Id$
//#

#include "gpvector.imp"
#include "rational.h"


#ifdef __GNUG__
// explicitly instantiate classes and non-member functions
template class gPVector<int>;
template class gPVector<double>;
template class gPVector<gRational>;

template gOutput & operator<< (gOutput&, const gPVector<int>&);
template gOutput & operator<< (gOutput&, const gPVector<double>&);
template gOutput & operator<< (gOutput&, const gPVector<gRational>&);
#elif defined(__BORLANDC__)
// whatever
#error Explicit instantiation needs to be done for Borland C++
#else
#error Unsupported compiler type.
#endif
