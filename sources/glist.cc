//#
//# FILE: glist.cc -- Provide implementations for commonly-used lists
//#
//# $Id$
//#

#include "gmisc.h"
#include "glist.imp"
#include "gstring.h"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gList<gRational>;
TEMPLATE class gNode<gRational>;

TEMPLATE class gList<gString>;
TEMPLATE class gNode<gString>;

