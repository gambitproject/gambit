//
// FILE: simpsub.cc -- Instantiation of SimpdivBySubgame
//
// $Id$
//

#include "rational.h"
#include "simpsub.imp"

int Simpdiv(const EFSupport &support, const SimpdivParams &params,
	    const gArray<gNumber> &values, gList<BehavSolution> &solutions,
	    int &nevals, int &/*niters*/, double &time)
{
  SimpdivBySubgame module(support, values, params);
  module.Solve();
  
  solutions = module.GetSolutions();
  nevals = module.NumEvals();
  time = module.Time();
  return 1;
}
