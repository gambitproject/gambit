//#
//# FILE: efgrat.cc -- Instantiation of rational-precision extensive forms
//#
//# $Id$
//#

#include "extform.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

DataType ExtForm<gRational>::Type(void) const   { return RATIONAL; }

TEMPLATE class TypedNode<gRational>;
TEMPLATE class ChanceInfoset<gRational>;
TEMPLATE class OutcomeVector<gRational>;
TEMPLATE class ExtForm<gRational>;
TEMPLATE int ReadEfgFile(gInput &, ExtForm<gRational> *&);
TEMPLATE class BehavProfile<gRational>;
TEMPLATE gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);

#include "glist.imp"

TEMPLATE class gList<BehavProfile<gRational> >;
TEMPLATE class gNode<BehavProfile<gRational> >;


