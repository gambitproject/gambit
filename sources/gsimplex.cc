//
// FILE: gsimplex.cc -- Instantiation of gInterval class
// 
// @(#)gsimplex.cc	1.1 01/07/98 
//

#include <assert.h>
#include "glist.imp"
#include "double.h"
#include "gsimplex.imp"
#include "glpsolv3.imp"


template class gSimplex<gRational>;
template class gList< gSimplex<gRational> >;
//template class gNode< gSimplex<gRational> >;
template gOutput& operator << (gOutput& output, const gSimplex<gRational>& x);
template gSimplex<gDouble> TogDouble(const gSimplex<gRational>&);

template class gSimplex<double>;
template class gList< gSimplex<double> >;
//template class gNode< gSimplex<double> >;
template gOutput& operator << (gOutput& output, const gSimplex<double>& x);
template gSimplex<gDouble> TogDouble(const gSimplex<double>&);

template class gSimplex<gDouble>;
template class gList< gSimplex<gDouble> >;
//template class gNode< gSimplex<gDouble> >;
template gOutput& operator << (gOutput& output, const gSimplex<gDouble>& x);
template gSimplex<gDouble> TogDouble(const gSimplex<gDouble>&);
