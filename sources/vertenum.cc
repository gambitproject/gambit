//#
//# FILE: vertenum.cc -- Instantiation of vertex enumerator
//#
//# $Id$
//#

#include "vertenum.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class VertEnum<double>;
TEMPLATE class VertEnum<gRational>;

