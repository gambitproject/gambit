//
// FILE: psnesub.cc -- Instantiation of PureNashBySubgame 
//
// $Id$
//

#include "rational.h"
#include "psnesub.imp"


int EnumPureNfg(const EFSupport &support, const gArray<gNumber> &values,
		gList<BehavSolution> &solutions, double &time)
{
  PureNashBySubgame module(support, values);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}
