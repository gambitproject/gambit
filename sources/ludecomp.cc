//#
//# FILE: ludecomp.cc -- Instantiation of LU decomposition classes
//#
//# $Id$
//#

#ifdef __GNUG__
#define TEMPLATE template
// this pragma is not necessary with g++ 2.6.3 /w -fno-implicit-TEMPLATEs
//#pragma implementation "gmatrix.h"
#elif defined(__BORLANDC__)
#pragma option -Jgd
#define TEMPLATE
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__

#include "ludecomp.imp"
#include "glist.imp"
#include "rational.h"

TEMPLATE class LUupdate<double>;
TEMPLATE class gList< LUupdate<double> >;
TEMPLATE class gNode< LUupdate<double> >;
TEMPLATE class gListIter< LUupdate<double> >;
TEMPLATE class LUdecomp<double>;

TEMPLATE gOutput& operator<<(gOutput&, LUupdate<double>);


TEMPLATE class LUupdate<gRational>;
TEMPLATE class gList< LUupdate<gRational> >;
TEMPLATE class gNode< LUupdate<gRational> >;
TEMPLATE class gListIter< LUupdate<gRational> >;
TEMPLATE class LUdecomp<gRational>;

TEMPLATE gOutput& operator<<(gOutput&, LUupdate<gRational>);
