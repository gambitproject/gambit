//#
//# FILE: gvector.cc -- Instantiation of vector types
//#
//# $Id$
//#

#ifdef __GNUG__
// this pragma is not necessary with g++ 2.6.3 /w -fno-implicit-templates
// #pragma implementation "gmatrix.h"
#elif defined(__BORLANDC__)
#pragma option -Jgd
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__

#include "gvector.imp"
#include "rational.h"


#ifdef __GNUG__
// explicitly instantiate classes and non-member functions
template class gVector<int>;
template class gVector<long>;
template class gVector<double>;
template class gVector<gInteger>;
template class gVector<gRational>;

template gOutput & operator<< (gOutput&, const gVector<int>&);
template gOutput & operator<< (gOutput&, const gVector<long>&);
template gOutput & operator<< (gOutput&, const gVector<double>&);
template gOutput & operator<< (gOutput&, const gVector<gInteger>&);
template gOutput & operator<< (gOutput&, const gVector<gRational>&);
#elif defined(__BORLANDC__)
// whatever
#error Explicit instantiation needs to be done for Borland C++
#else
#error Unsupported compiler type.
#endif
