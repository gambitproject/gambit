//
// FILE: efgcsum.cc -- Constant Sum Extensive Form Game Solution Module
//
// $Id$
//

#include "efgcsum.imp"
#include "rational.h"

//---------------------------------------------------------------------------
//                        EFCSumParams: member functions
//---------------------------------------------------------------------------

CSSeqFormParams::CSSeqFormParams(gStatus &status_) 
  :  trace(0), stopAfter(0), precision(precDOUBLE),
     tracefile(&gnull), status(status_)
{ }

int _CSSeqForm(const EFSupport &support, const CSSeqFormParams &params,
	       gList<BehavSolution> &solutions, int &npivots, double &time)
{
  if (params.precision == precDOUBLE)   {
    CSSeqFormModule<double> module(support, params);
    module.CSSeqForm();
    npivots = module.NumPivots();
    time = module.Time();
    module.GetSolutions(solutions);
    return 1;
  }
  else if (params.precision == precRATIONAL)  {
    CSSeqFormModule<gRational> module(support, params);
    module.CSSeqForm();
    npivots = module.NumPivots();
    time = module.Time();
    module.GetSolutions(solutions);
    return 1;
  }
  return 1;
}    

int CSSeqForm(const EFSupport &support, const CSSeqFormParams &params,
	      gList<BehavSolution> &solutions, int &npivots, double &time)
{
  CSSeqFormBySubgame module(support, params);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

template class CSSeqFormModule<double>;
template class CSSeqFormModule<gRational>;








