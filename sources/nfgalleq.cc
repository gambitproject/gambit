//
// FILE: nfgalleq.cc -- All Nash Enum module
//
// $Id$
//

#include "nfgalleq.imp"

int AllNashSolve(const Nfg &N, const PolEnumParams &params,
	       gList<MixedSolution> &solutions, long &nevals, double &time)
{
  if (params.precision == precDOUBLE)  {
    AllNashSolveModule<gDouble> module(N, params);
    module.NashEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    AllNashSolveModule<gRational> module(N, params);
    module.NashEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

template class AllNashSolveModule<double>;
template class AllNashSolveModule<gDouble>;
template class AllNashSolveModule<gRational>;











