//#
//# FILE: ludecomp.cc -- Instantiation of LU decomposition classes
//#
//# $Id$
//#

#ifdef __GNUG__
// this pragma is not necessary with g++ 2.6.3 /w -fno-implicit-templates
//#pragma implementation "gmatrix.h"
#elif defined(__BORLANDC__)
#pragma option -Jgd
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__

#include "ludecomp.imp"
#include "glist.imp"
#include "rational.h"

template class LUupdate<double>;
template class gList< LUupdate<double> >;
template class gNode< LUupdate<double> >;
template class gListIter< LUupdate<double> >;
template class LUdecomp<double>;

template gOutput& operator<<(gOutput&, LUupdate<double>);


template class LUupdate<gRational>;
template class gList< LUupdate<gRational> >;
template class gNode< LUupdate<gRational> >;
template class gListIter< LUupdate<gRational> >;
template class LUdecomp<gRational>;

template gOutput& operator<<(gOutput&, LUupdate<gRational>);
