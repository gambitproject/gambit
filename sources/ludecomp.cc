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
#include "rational.h"

template class LUdecomp<double>;
template class LUdecomp<gRational>;

template class LUupdate<double>;
template class LUupdate<gRational>;
