//
// FILE: garray.cc -- Instantiations for common gArray classes
//
// $Id$
//

#include "base/base.h"
#include "base/garray.imp"

#include "rational.h"
#include "gnumber.h"
#include "complex.h"

template class gArray<bool>;
template class gArray<int>;
template class gArray<long>;
template class gArray<float>;
template class gArray<double>;
template class gArray<gInteger>;
template class gArray<gRational>;
template class gArray<gComplex>;
template class gArray<gNumber>;
template class gArray<gText>;
template class gArray<gArray<int> >;
template class gArray<gArray<int> *>;
template class gArray<gArray<double> >;
template class gArray<void *>;
template class gArray<gList<bool> >;
template class gArray<gList<gList<bool> > >;
template bool operator==(const gArray<int> &, const gArray<int> &);
template bool operator!=(const gArray<int> &, const gArray<int> &);
template bool operator==(const gArray<gList<bool> > &, 
			 const gArray<gList<bool> > &);
template bool operator!=(const gArray<gList<bool> > &, 
			 const gArray<gList<bool> > &);
template bool operator==(const gArray<gList<gList<bool> > > &, 
			 const gArray<gList<gList<bool> > > &);
template bool operator!=(const gArray<gList<gList<bool> > > &, 
			 const gArray<gList<gList<bool> > > &);

template gOutput &operator<<(gOutput &, const gArray<bool> &);
template gOutput &operator<<(gOutput &, const gArray<int> &);
template gOutput &operator<<(gOutput &, const gArray<long> &);
template gOutput &operator<<(gOutput &, const gArray<float> &);
template gOutput &operator<<(gOutput &, const gArray<double> &);
template gOutput &operator<<(gOutput &, const gArray<gInteger> &);
template gOutput &operator<<(gOutput &, const gArray<gRational> &);
template gOutput &operator<<(gOutput &, const gArray<gText> &);
template gOutput &operator<<(gOutput &, const gArray<gArray<int> > &);
template gOutput &operator<<(gOutput &, const gArray<gArray<int> *> &);
template gOutput &operator<<(gOutput &, const gArray<gArray<double> > &);

