//
// FILE: simpdiv.cc -- Mixed strategy algorithm for Gambit
//
// $Id$
//

#include "simpdiv.imp"

SimpdivParams::SimpdivParams(gStatus &status_)
  : trace(0), stopAfter(1), nRestarts(20), leashLength(0), 
    precision(precDOUBLE), tracefile(&gnull), status(status_)
{ }

int Simpdiv(const NFSupport &support, const SimpdivParams &params,
	    const gArray<gNumber> &values, gList<MixedSolution> &solutions,
	    int &nevals, int &niters, double &time)
{
  if (params.precision == precDOUBLE)  {
    SimpdivModule<double> module(support, params, values);
    module.Simpdiv();
    nevals = module.NumEvals();
    niters = module.NumIters();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SimpdivModule<gRational> module(support, params, values);
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

