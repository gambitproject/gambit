//
// FILE: polytope.cc -- Instantiation of gPolytope class
//
// @(#)polytope.cc	1.27 06/13/97
//

#include "rational.h"
#include "dualtope.imp"
#include "glist.imp"

template class gHalfSpc<gRational>;
template gOutput &operator<<(gOutput&, const gHalfSpc<gRational>&);

template class gList<gHalfSpc<gRational> >;
template class gNode<gHalfSpc<gRational> >;

template class gDualTope<gRational>;
template gOutput &operator<<(gOutput&, const gDualTope<gRational>&);






