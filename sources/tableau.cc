//#
//# FILE: tableau.cc -- tableau and basis class instantiations
//#
//# $Id$
//#

#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#else
#error Unsupported compiler type
#endif // __GNUG__, __BORLANDC__

TEMPLATE class Tableau<double>;
TEMPLATE class Tableau<gRational>;

TEMPLATE class Basis<double>;
TEMPLATE class Basis<gRational>;

TEMPLATE class BasisCode<double>;
TEMPLATE class BasisCode<gRational>;
