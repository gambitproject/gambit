//
// FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
//
// $Id$
//

#include "nfg.imp"

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

#include "glist.imp"

template class gList<MixedProfile<double> >;
template class gNode<MixedProfile<double> >;

#include "grblock.imp"

template class gRectBlock<double>;

