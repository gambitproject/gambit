//
// FILE: gdpvect.cc -- Instantiation of doubly partitioned vector types
//
// $Id$
//

#include "base/base.h"
#include "math/gdpvect.imp"
#include "math/gnumber.h"


template class gDPVector<double>;
template class gDPVector<gRational>;
template class gDPVector<gNumber>;

template gOutput & operator<< (gOutput&, const gDPVector<double>&);
template gOutput & operator<< (gOutput&, const gDPVector<gRational>&);
template gOutput & operator<< (gOutput&, const gDPVector<gNumber>&);

