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

int SeqForm(const EFSupport &support, const SeqFormParams &params,
	    gList<BehavSolution> &solutions, int &npivots, double &time)
{
  if (params.precision == precDOUBLE)  {
    SeqFormBySubgame<double> module(support.Game(), support, params);
    module.Solve();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SeqFormBySubgame<gRational> module(support.Game(), support, params);
    module.Solve();
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

template class SeqFormModule<double>;
template class SeqFormModule<gRational>;
template class SeqFormBySubgame<double>;
template class SeqFormBySubgame<gRational>;












