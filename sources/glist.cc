//#
//# FILE: glist.cc -- Provide implementations for commonly-used lists
//#
//# $Id$
//#

#include "gmisc.h"
#include "glist.imp"
#include "gstring.h"
#include "rational.h"

#include "garray.h"
#include "gblock.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gList<gRational>;
TEMPLATE class gNode<gRational>;

TEMPLATE class gList<int>;
TEMPLATE class gNode<int>;

TEMPLATE class gList<long>;
TEMPLATE class gNode<long>;

TEMPLATE class gList<double>;
TEMPLATE class gNode<double>;

TEMPLATE class gList<gString>;
TEMPLATE class gNode<gString>;

TEMPLATE class gList<gBlock<int> >;
TEMPLATE class gNode<gBlock<int> >;

TEMPLATE class gList<gArray<int> >;
TEMPLATE class gListIter<gArray<int> >;
TEMPLATE class gNode<gArray<int> >;



