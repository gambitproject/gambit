//
// FILE: gvector.cc -- Instantiation of vector types
//
// $Id$
//

#include "gvector.imp"
#include "gnumber.h"



template class gVector<int>;
template class gVector<long>;
template class gVector<double>;
template class gVector<gInteger>;
template class gVector<gRational>;
template class gVector<gNumber>;

template gOutput & operator<< (gOutput&, const gVector<int>&);
template gOutput & operator<< (gOutput&, const gVector<long>&);
template gOutput & operator<< (gOutput&, const gVector<double>&);
template gOutput & operator<< (gOutput&, const gVector<gInteger>&);
template gOutput & operator<< (gOutput&, const gVector<gRational>&);
template gOutput & operator<< (gOutput&, const gVector<gNumber>&);

