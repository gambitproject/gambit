//
// FILE: gpvector.cc -- Instantiation of partitioned vector types
//
// $Id$
//

#include "base/base.h"
#include "gpvector.imp"
#include "rational.h"
#include "gnumber.h"

template class gPVector<int>;
template class gPVector<double>;
template class gPVector<gRational>;
template class gPVector<gNumber>;

template gOutput & operator<< (gOutput&, const gPVector<int>&);
template gOutput & operator<< (gOutput&, const gPVector<double>&);
template gOutput & operator<< (gOutput&, const gPVector<gRational>&);
template gOutput & operator<< (gOutput&, const gPVector<gNumber>&);
