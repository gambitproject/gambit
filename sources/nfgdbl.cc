//
// FILE: nfginst.cc -- Instantiation of normal form template classes
//
// 
//

#include "mixed.imp"

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

#include "glist.imp"

template class gList<MixedProfile<double> >;

#include "grblock.imp"

template class gRectBlock<double>;

