//
// FILE: seqform.cc -- Sequence Form module
//
// $Id$ 
//

#include "seqform.imp"
#include "math/rational.h"

//---------------------------------------------------------------------------
//                        SeqFormParams: member functions
//---------------------------------------------------------------------------

SeqFormParams::SeqFormParams(void)
{ }

int _SeqForm(const EFSupport &support, const SeqFormParams &params,
	     gList<BehavSolution> &solutions, gStatus &p_status,
	     int &npivots, double &time)
{
  if (params.precision == precDOUBLE)  {
    SeqFormModule<double> module(support, params);
    module.Lemke(p_status);
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SeqFormModule<gRational> module(support, params);
    module.Lemke(p_status);
    npivots = module.NumPivots();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

template class SeqFormModule<double>;
template class SeqFormModule<gRational>;












