//
// FILE: nfgpure.cc -- Find all pure strategy Nash equilibria
//
// @(#)nfgpure.cc	2.3 6/22/97
//

#include "nfgpure.imp"
#include "rational.h"


template int FindPureNash(const Nfg &, const NFSupport &,
			  gList<MixedSolution<double> > &);
template int FindPureNash(const Nfg &, const NFSupport &,
			  gList<MixedSolution<gRational> > &);

