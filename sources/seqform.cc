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


template class SeqFormModule<double>;
template class SeqFormModule<gRational>;












