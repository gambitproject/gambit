//
// FILE: gvector.cc -- Instantiation of vector types
//
// $Id$
//

#include "gvector.imp"
#include "gnumber.h"
#include "complex.h"

template class gVector<int>;
template class gVector<long>;
template class gVector<double>;
template class gVector<long double>;
template class gVector<gInteger>;
template class gVector<gRational>;
template class gVector<gComplex>;
template class gVector<gNumber>;

template gOutput & operator<< (gOutput&, const gVector<int>&);
template gOutput & operator<< (gOutput&, const gVector<long>&);
template gOutput & operator<< (gOutput&, const gVector<double>&);
template gOutput & operator<< (gOutput&, const gVector<gInteger>&);
template gOutput & operator<< (gOutput&, const gVector<gRational>&);
template gOutput & operator<< (gOutput&, const gVector<gComplex>&);
template gOutput & operator<< (gOutput&, const gVector<gNumber>&);

template gVector<gDouble> TogDouble(const gVector<gRational>&);
template gVector<gDouble> TogDouble(const gVector<double>&);
template gVector<gDouble> TogDouble(const gVector<gDouble>&);

#include "base/glist.imp"

template class gList<gVector<double> >;
template class gList<gVector<gRational> >;
template class gList<gVector<gComplex> >;
template gOutput& operator << (gOutput& output, 
			       const gList<gVector<gRational> >&);


