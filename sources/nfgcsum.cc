//
// FILE: nfgcsum.cc -- Constant Sum Game Solution Module
//
// $Id$
//

#include "nfgcsum.imp"
#include "rational.h"

//---------------------------------------------------------------------------
//                        ZSumParams: member functions
//---------------------------------------------------------------------------

ZSumParams::ZSumParams(gStatus &status_) 
  :  trace(0), stopAfter(0), precision(precDOUBLE),
     tracefile(&gnull), status(status_)
{ }

int ZSum(const NFSupport &support, const ZSumParams &params, 
	 const gArray<gNumber> &values,
	 gList<MixedSolution> &solutions,
	 int &npivots, double &time)
{
  if (params.precision == precDOUBLE)  {
    ZSumModule<double> module(support, params, values);
    module.ZSum();
    module.GetSolutions(solutions);
    npivots = module.NumPivots();
    time = module.Time();
  }
  else  {
    ZSumModule<gRational> module(support, params, values);
    module.ZSum();
    module.GetSolutions(solutions);
    npivots = module.NumPivots();
    time = module.Time();
  }

  return 1;
}

template class ZSumModule<double>;
template class ZSumModule<gRational>;

