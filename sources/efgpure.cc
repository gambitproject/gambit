//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.imp"
#include "rational.h"

int EnumPure(const EFSupport &support, const gArray<gNumber> &values,
	     gList<BehavSolution> &solutions, double &time)
{
  EfgPSNEBySubgame module(support, values);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}


