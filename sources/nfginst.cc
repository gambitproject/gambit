//
// FILE: nfginst.cc -- Instantiation of normal form template classes
//
// 
//

#include "mixed.imp"
#include "rational.h"

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

template class MixedProfile<gRational>;
template gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

template class MixedProfile<gNumber>;
template gOutput &operator<<(gOutput &, const MixedProfile<gNumber> &);


#include "glist.imp"

template class gList<MixedProfile<double> >;
template class gList<MixedProfile<gRational> >;
template class gList<MixedProfile<gNumber> >;

#include "grblock.imp"

template class gRectBlock<double>;
template class gRectBlock<gRational>;

#include "garray.imp"
#include "gblock.imp"

template class gArray<nfgSupportPlayer *>;
template class gArray<Strategy *>;
template class gArray<NFOutcome *>;

template class gArray<NFPlayer *>;
template class gBlock<Strategy *>;

template class gBlock<NFOutcome *>;

template class gList<MixedSolution>;
template class gList<Strategy *>;

template class gList<const NFSupport>;


