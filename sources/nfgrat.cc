//
// FILE: nfgrat.cc -- Instantiation of rational-precision normal forms
//
// $Id$
//

#include "rational.h"
#include "nfg.imp"

template class MixedProfile<gRational>;
template gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

#include "glist.imp"

template class gList<MixedProfile<gRational> >;

#include "grblock.imp"

template class gRectBlock<gRational>;
