//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.imp"
#include "rational.h"

int EnumPure(const EFSupport &support, gList<BehavSolution> &solutions, 
	     double &time)
{
  EfgPSNEBySubgame<double> module(support.Game(), support);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

template class EfgPSNEBySubgame<double>;
template class EfgPSNEBySubgame<gRational>;

