//#
//# FILE: efgrat.cc -- Instantiation of rational-precision extensive forms
//#
//# $Id$
//#

#include "rational.h"
#include "extform.h"
#include "glist.h"
#include "glistit.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class BehavProfile<double>;
class ExtForm<double>;
class gList<Node *>;
class gListIter<Node *>;
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "extform.imp"

TEMPLATE class ExtForm<gRational>;
DataType ExtForm<gRational>::Type(void) const   { return RATIONAL; }

TEMPLATE class TypedNode<gRational>;
TEMPLATE class ChanceInfoset<gRational>;
TEMPLATE class OutcomeVector<gRational>;
TEMPLATE int ReadEfgFile(gInput &, ExtForm<gRational> *&);
TEMPLATE class BehavProfile<gRational>;
TEMPLATE gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);
//TEMPLATE bool operator==(const gArray<gRational> &, const gArray<gRational> &);
//TEMPLATE bool operator!=(const gArray<gRational> &, const gArray<gRational> &);
#include "glist.imp"

TEMPLATE class gList<BehavProfile<gRational> >;
TEMPLATE class gNode<BehavProfile<gRational> >;


