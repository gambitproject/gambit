//
// FILE: psnesub.cc -- Instantiation of PureNashBySubgame 
//
// $Id$
//

#include "rational.h"
#include "psnesub.imp"


int EnumPureNfg(const EFSupport &support, gList<BehavSolution> &solutions, 
		double &time)
{
  PureNashBySubgame<double> module(support.Game(), support);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

template class PureNashBySubgame<double>;
template class PureNashBySubgame<gRational>;
