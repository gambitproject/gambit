//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "rational.h"
#include "enumsub.imp"

int Enum(const EFSupport &support, const EnumParams &params,
	 gList<BehavSolution> &solutions, long &npivots, double &time)
{
  if (params.precision == precDOUBLE)   {
    EnumBySubgame<double> module(support.Game(), support, params);
    module.Solve();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    EnumBySubgame<gRational> module(support.Game(), support, params);
    module.Solve();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  
  return 1;
}


template class EnumBySubgame<double>;
template class EnumBySubgame<gRational>;
