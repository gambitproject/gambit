//#
//# FILE: gdpvect.cc -- Instantiation of doubly partitioned vector types
//#
//# $Id$
//#

#include "gdpvect.imp"
#include "rational.h"


template class gDPVector<double>;
template class gDPVector<gRational>;

template gOutput & operator<< (gOutput&, const gDPVector<double>&);
template gOutput & operator<< (gOutput&, const gDPVector<gRational>&);
