//#
//# FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
//#
//# $Id$
//#

#include "rational.h"
#include "normal.h"
#include "glist.h"
#include "glistit.h"
#include "gpset.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class MixedProfile<gRational>;
class NormalForm<gRational>;

#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "normal.imp"
#include "normiter.imp"
#include "contiter.imp"
#include "gnarray.imp"

TEMPLATE class gNArray<double>;

TEMPLATE class NormalForm<double>;
DataType NormalForm<double>::Type(void) const    { return DOUBLE; }

TEMPLATE class NormalIter<double>;
TEMPLATE class ContIter<double>;

TEMPLATE class MixedProfile<double>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<double> &);

#include "glist.imp"

TEMPLATE class gList<MixedProfile<double> >;
TEMPLATE class gNode<MixedProfile<double> >;



