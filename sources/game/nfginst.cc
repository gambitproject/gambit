//
// FILE: nfginst.cc -- Instantiation of normal form template classes
//
// $Id$ 
//

#include "behav.h"
#include "lexicon.h"
#include "mixed.imp"
#include "math/rational.h"

TEMPLATE_SPECIALIZATION()
const gNumber MixedProfile<gNumber>::Payoff(const NFOutcome *o, int pl) const
{ 
  return o->payoffs[pl];
}

TEMPLATE_SPECIALIZATION()
const gRational MixedProfile<gRational>::Payoff(const NFOutcome *o, int pl) const
{ 
  gout << "\nin MixedProfile<gRational>::Payoff()";
  return o->payoffs[pl];
}

TEMPLATE_SPECIALIZATION()
const double MixedProfile<double>::Payoff(const NFOutcome *o, int pl) const
{ 
  return o->double_payoffs[pl];
}

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

template class MixedProfile<gRational>;
template gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

template class MixedProfile<gNumber>;
template gOutput &operator<<(gOutput &, const MixedProfile<gNumber> &);


#include "base/glist.imp"

template class gList<MixedProfile<double> >;
template class gList<MixedProfile<gRational> >;
template class gList<MixedProfile<gNumber> >;

#include "base/grblock.imp"

template class gRectBlock<double>;
template class gRectBlock<gRational>;

#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<nfgSupportPlayer *>;
template class gArray<Strategy *>;
template class gArray<NFOutcome *>;

template class gArray<NFPlayer *>;
template class gBlock<Strategy *>;

template class gBlock<NFOutcome *>;

template class gList<MixedSolution>;
template class gList<Strategy *>;

template class gList<const NFSupport>;


#include "corresp.imp"

template class Correspondence<double, MixedSolution>;

