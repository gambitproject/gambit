//
// FILE: simpdiv.cc -- Mixed strategy algorithm for Gambit
//
// $Id$
//

#include "simpdiv.imp"

SimpdivParams::SimpdivParams(gStatus &s)
  : AlgParams(s), nRestarts(20), leashLength(0)
{ }

int Simpdiv(const NFSupport &support, const SimpdivParams &params,
	    gList<MixedSolution> &solutions,
	    int &nevals, int &niters, double &time)
{
  if (params.precision == precDOUBLE)  {
    SimpdivModule<double> module(support, params);
    module.Simpdiv();
    nevals = module.NumEvals();
    niters = module.NumIters();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SimpdivModule<gRational> module(support, params);
    module.Simpdiv();
    nevals = module.NumEvals();
    niters = module.NumIters();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}  



#include "rational.h"

template class SimpdivModule<double>;
template class SimpdivModule<gRational>;

