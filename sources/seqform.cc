//
// FILE: seqform.cc -- Sequence Form module
//
// $Id$ 
//

#include "seqform.imp"
#include "rational.h"

//---------------------------------------------------------------------------
//                        SeqFormParams: member functions
//---------------------------------------------------------------------------

SeqFormParams::SeqFormParams(gStatus &status_) 
  : trace(0), stopAfter(0), precision(precDOUBLE),
    tracefile(&gnull), status(status_)
{ }

int _SeqForm(const EFSupport &support, const SeqFormParams &params,
	     gList<BehavSolution> &solutions, int &npivots, double &time)
{
  if (params.precision == precDOUBLE)  {
    SeqFormModule<double> module(support, params);
    module.Lemke();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SeqFormModule<gRational> module(support, params);
    module.Lemke();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

int efgLcpSolve::SolveSubgame(const Efg &/*E*/, const EFSupport &sup,
			      gList<BehavSolution> &solns)
{
  int npiv;
  double time;
  _SeqForm(sup, params, solns, npiv, time);

  npivots += npiv;

  return 1;
}

efgLcpSolve::efgLcpSolve(const EFSupport &S,
			 const SeqFormParams &p, int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

efgLcpSolve::~efgLcpSolve()   { }


template class SeqFormModule<double>;
template class SeqFormModule<gRational>;












