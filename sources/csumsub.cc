//
// FILE: csumsub.cc -- Instantiation of ZSumBySubgame
//
// $Id$
//

#include "rational.h"
#include "csumsub.imp"

int ZSum(const EFSupport &support, const ZSumParams &params,
	 const gArray<gNumber> &values,
	 gList<BehavSolution> &solutions, int &npivots, double &time)
{
  ZSumBySubgame module(support, params, values);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

