//
// FILE: polensub.cc -- Instantiation of PolEnumBySubgame
//
// $Id$ 
//

#include "rational.h"
#include "polensub.imp"

int PolEnum(const EFSupport &support, const PolEnumParams &params,
	 const gArray<gNumber> &values, 
	 gList<BehavSolution> &solutions, long &nevals, double &time)
{
  PolEnumBySubgame module(support, values, params);
  module.Solve();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

