//
// FILE: gsmpltpe.cc -- Instantiation of gSimpletope class
// 
// $Id$
//

#include <assert.h>
#include "glist.imp"
#include "double.h"
#include "gsmpltpe.imp"
#include "glpsolv3.imp"


template class gSimpletope<gRational>;
template class gList< gSimpletope<gRational> >;
template gOutput& operator << (gOutput& output, 
			       const gSimpletope<gRational>& x);
template gSimpletope<gDouble> TogDouble(const gSimpletope<gRational>&);

template class gSimpletope<double>;
template class gList< gSimpletope<double> >;
template gOutput& operator << (gOutput& output, 
			       const gSimpletope<double>& x);
template gSimpletope<gDouble> TogDouble(const gSimpletope<double>&);

template class gSimpletope<gDouble>;
template class gList< gSimpletope<gDouble> >;
template gOutput& operator << (gOutput& output, 
			       const gSimpletope<gDouble>& x);
template gSimpletope<gDouble> TogDouble(const gSimpletope<gDouble>&);
