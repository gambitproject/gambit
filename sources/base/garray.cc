//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations for common gArray classes
//

#include "base.h"
#include "garray.imp"

template class gArray<bool>;
template class gArray<int>;
template class gArray<long>;
template class gArray<float>;
template class gArray<double>;
template class gArray<long double>;
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
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gArray<int> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gArray<long> &);
template gOutput &operator<<(gOutput &, const gArray<float> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gArray<double> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gArray<gArray<int> > &);
template gOutput &operator<<(gOutput &, const gArray<gArray<int> *> &);
template gOutput &operator<<(gOutput &, const gArray<gArray<double> > &);

