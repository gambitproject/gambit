//#
//# FILE: garray.cc -- Instantiations for common gArray classes
//#
//# $Id$
//#

#include "garray.imp"
#include "gmisc.h"
#include "gstring.h"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gArray<bool>;
TEMPLATE class gArray<int>;
TEMPLATE class gArray<long>;
TEMPLATE class gArray<float>;
TEMPLATE class gArray<double>;
TEMPLATE class gArray<gInteger>;
TEMPLATE class gArray<gRational>;
TEMPLATE class gArray<gString>;
TEMPLATE class gArray<gArray<int> >;
TEMPLATE class gArray<gArray<int> *>;
TEMPLATE class gArray<gArray<double> >;
TEMPLATE class gArray<void *>;
TEMPLATE bool operator==(const gArray<int> &, const gArray<int> &);
TEMPLATE bool operator!=(const gArray<int> &, const gArray<int> &);

/* commented out for now

TEMPLATE class gArrayPtr<bool>;
TEMPLATE class gArrayPtr<int>;
TEMPLATE class gArrayPtr<long>;
TEMPLATE class gArrayPtr<float>;
TEMPLATE class gArrayPtr<double>;
TEMPLATE class gArrayPtr<gInteger>;
TEMPLATE class gArrayPtr<gRational>;
TEMPLATE class gArrayPtr<gString>;
*/

TEMPLATE gOutput &operator<<(gOutput &, const gArray<bool> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<int> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<long> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<float> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gInteger> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gRational> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gString> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gArray<int> > &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gArray<int> *> &);
TEMPLATE gOutput &operator<<(gOutput &, const gArray<gArray<double> > &);

