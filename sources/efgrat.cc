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
