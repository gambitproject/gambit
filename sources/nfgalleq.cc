//
// FILE: nfgalleq.cc -- All Nash Enum module 
//
// $Id$
//

#include "nfgalleq.imp"

//---------------------------------------------------------------------------
//                            AllNashSolve
//---------------------------------------------------------------------------

int AllNashSolve(const Nfg &N, const PolEnumParams &params,
		 gList<MixedSolution> &solutions, long &nevals, double &time,
		 gList<const NFSupport> &singular_supports)
{
  if (params.precision == precDOUBLE)  {
    AllNashSolveModule<gDouble> module(N, params);
    module.NashEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
    singular_supports = module.GetSingularSupports();
  }
  else if (params.precision == precRATIONAL)  {
    AllNashSolveModule<gRational> module(N, params);
    module.NashEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
    singular_supports = module.GetSingularSupports();
  }

  return 1;
}

template class AllNashSolveModule<double>;
template class AllNashSolveModule<gDouble>;
template class AllNashSolveModule<gRational>;











