//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.imp"
#include "rational.h"


template int FindPureNash(const Efg &, const EFSupport &,
			  gList<BehavSolution<double> > &);
template int FindPureNash(const Efg &, const EFSupport &,
			  gList<BehavSolution<gRational> > &);


template class EfgPSNEBySubgame<double>;
template class EfgPSNEBySubgame<gRational>;

