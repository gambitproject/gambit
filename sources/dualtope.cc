//
// FILE: polytope.cc -- Instantiation of gPolytope class
//
// $Id$
//

#include "rational.h"
#include "dualtope.imp"
#include "glist.imp"

template class gHalfSpc<gRational>;
template gOutput &operator<<(gOutput&, const gHalfSpc<gRational>&);

template class gList<gHalfSpc<gRational> >;

template class gDualTope<gRational>;
template gOutput &operator<<(gOutput&, const gDualTope<gRational>&);






