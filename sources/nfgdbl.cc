//
// FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
//
// $Id$
//

#include "nfg.imp"
#include "mixedsol.imp"

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

template class MixedSolution<double>;
template gOutput &operator<<(gOutput &, const MixedSolution<double> &);

#include "glist.imp"

template class gList<MixedProfile<double> >;
template class gNode<MixedProfile<double> >;

template class gList<MixedSolution<double> >;
template class gNode<MixedSolution<double> >;

#include "grblock.imp"

template class gRectBlock<double>;

