//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "rational.h"
#include "enumsub.imp"

int Enum(const EFSupport &support, const EnumParams &params,
	 const gArray<gNumber> &values, 
	 gList<BehavSolution> &solutions, long &npivots, double &time)
{
  EnumBySubgame module(support, values, params);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

