//#
//# FILE: efgdbl.cc -- Instantiation of double-precision extensive forms
//#
//# $Id$
//#

#include "extform.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

DataType ExtForm<double>::Type(void) const    { return DOUBLE; }

TEMPLATE class TypedNode<double>;
TEMPLATE class ChanceInfoset<double>;
TEMPLATE class OutcomeVector<double>;
TEMPLATE class ExtForm<double>;


