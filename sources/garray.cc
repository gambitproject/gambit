//
// FILE: garray.cc -- Instantiations for common gArray classes
//
// $Id$
//

#include "garray.imp"
#include "gmisc.h"
#include "gtext.h"
#include "rational.h"
#include "gnumber.h"
#include "glist.h"

template class gArray<bool>;
template class gArray<int>;
template class gArray<long>;
template class gArray<float>;
template class gArray<double>;
template class gArray<gInteger>;
template class gArray<gRational>;
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

/* commented out for now

template class gArrayPtr<bool>;
template class gArrayPtr<int>;
template class gArrayPtr<long>;
template class gArrayPtr<float>;
template class gArrayPtr<double>;
template class gArrayPtr<gInteger>;
template class gArrayPtr<gRational>;
template class gArrayPtr<gString>;
*/

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

