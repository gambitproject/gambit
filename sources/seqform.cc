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

int _SeqForm(const EFSupport &support, const gArray<gNumber> &values,
	     const SeqFormParams &params,
	     gList<BehavSolution> &solutions, int &npivots, double &time)
{
  if (params.precision == precDOUBLE)  {
    SeqFormModule<double> module(support, values, params);
    module.Lemke();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SeqFormModule<gRational> module(support, values, params);
    module.Lemke();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

int SeqForm(const EFSupport &support, const gArray<gNumber> &values,
	    const SeqFormParams &params, gList<BehavSolution> &solutions,
	    int &npivots, double &time)
{
  SeqFormBySubgame module(support, values, params);
  module.Solve();
  solutions = module.GetSolutions();
  npivots = module.NumPivots();
  time = module.Time();
  return 1;
}

int SeqFormBySubgame::SolveSubgame(const Efg &/*E*/, const EFSupport &sup,
				   gList<BehavSolution> &solns)
{
  int npiv;
  double time;
  _SeqForm(sup, values, params, solns, npiv, time);

  npivots += npiv;

  return 1;
}

SeqFormBySubgame::SeqFormBySubgame(const EFSupport &S,
				   const gArray<gNumber> &v,
				   const SeqFormParams &p, int max)
  : SubgameSolver(S, v, max), npivots(0), params(p), values(v)
{ }

SeqFormBySubgame::~SeqFormBySubgame()   { }


template class SeqFormModule<double>;
template class SeqFormModule<gRational>;












