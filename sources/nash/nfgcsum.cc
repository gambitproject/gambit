//
// FILE: nfgcsum.cc -- Constant Sum Game Solution Module
//
// $Id$
//

#include "nfgcsum.imp"
#include "math/rational.h"

//---------------------------------------------------------------------------
//                        ZSumParams: member functions
//---------------------------------------------------------------------------

ZSumParams::ZSumParams(void)
{ }

int ZSum(const NFSupport &support, const ZSumParams &params, 
	 gList<MixedSolution> &solutions, gStatus &p_status,
	 int &npivots, double &time)
{
  if (params.precision == precDOUBLE)  {
    ZSumModule<double> module(support, params);
    module.ZSum(p_status);
    module.GetSolutions(solutions);
    npivots = module.NumPivots();
    time = module.Time();
  }
  else  {
    ZSumModule<gRational> module(support, params);
    module.ZSum(p_status);
    module.GetSolutions(solutions);
    npivots = module.NumPivots();
    time = module.Time();
  }

  return 1;
}

template class ZSumModule<double>;
template class ZSumModule<gRational>;

