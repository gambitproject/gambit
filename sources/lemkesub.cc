//
// FILE: lemkesub.cc -- Instantiation of LemkeBySubgame
//
// $Id$
//

#include "rational.h"
#include "lemkesub.imp"

int Lemke(const EFSupport &support, const LemkeParams &params,
	  gList<BehavSolution> &solutions, int &npivots, double &time)
{
  if (params.precision == precDOUBLE)   {
    LemkeBySubgame<double> module(support.Game(), support, params);
    module.Solve();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    LemkeBySubgame<gRational> module(support.Game(), support, params);
    module.Solve();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  
  return 1;
}

template class LemkeBySubgame<double>;
template class LemkeBySubgame<gRational>;
