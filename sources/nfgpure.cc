//
// FILE: nfgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "nfgpure.imp"
#include "rational.h"


template int FindPureNash(const Nfg<double> &, const NFSupport &,
			  gList<MixedSolution<double> > &);
template int FindPureNash(const Nfg<gRational> &, const NFSupport &,
			  gList<MixedSolution<gRational> > &);

