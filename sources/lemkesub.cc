//
// FILE: lemkesub.cc -- Instantiation of LemkeBySubgame
//
// $Id$
//

#include "rational.h"
#include "lemkesub.imp"

int Lemke(const EFSupport &support, const LemkeParams &params,
	  const gArray<gNumber> &values,
	  gList<BehavSolution> &solutions, int &npivots, double &time)
{
  LemkeBySubgame module(support, params, values);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}
