//#
//# FILE: gpvector.cc -- Instantiation of partitioned vector types
//#
//# $Id$
//#

#include "gpvector.imp"
#include "rational.h"


template class gPVector<int>;
template class gPVector<double>;
template class gPVector<gRational>;

template gOutput & operator<< (gOutput&, const gPVector<int>&);
template gOutput & operator<< (gOutput&, const gPVector<double>&);
template gOutput & operator<< (gOutput&, const gPVector<gRational>&);
