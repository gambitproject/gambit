//#
//# FILE: nfgrat.cc -- Instantiation of rational normal forms
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
class MixedProfile<double>;
class NormalForm<double>;

#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "normal.imp"
#include "normiter.imp"
#include "contiter.imp"
#include "gnarray.imp"

TEMPLATE class gNArray<gRational>;

TEMPLATE class NormalForm<gRational>;
DataType NormalForm<gRational>::Type(void) const   { return RATIONAL; }

TEMPLATE class NormalIter<gRational>;
TEMPLATE class ContIter<gRational>;

TEMPLATE class MixedProfile<gRational>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

#include "glist.imp"

TEMPLATE class gList<MixedProfile<gRational> >;
TEMPLATE class gNode<MixedProfile<gRational> >;
