//
// FILE: efgcsum.cc -- Constant Sum Extensive Form Game Solution Module
//
// $Id$
//

#include "efgcsum.imp"
#include "math/rational.h"

//---------------------------------------------------------------------------
//                        EFCSumParams: member functions
//---------------------------------------------------------------------------

CSSeqFormParams::CSSeqFormParams(void) 
{ }


//-----------------------------------
// Interfacing to solve-by-subgame
//-----------------------------------

static int _CSSeqForm(const EFSupport &support, const CSSeqFormParams &params,
		      gList<BehavSolution> &solutions, gStatus &p_status,
		      int &npivots, double &time)
{
  if (params.precision == precDOUBLE)   {
    CSSeqFormModule<double> module(support, params);
    module.CSSeqForm(p_status);
    npivots = module.NumPivots();
    time = module.Time();
    module.GetSolutions(solutions);
    return 1;
  }
  else if (params.precision == precRATIONAL)  {
    CSSeqFormModule<gRational> module(support, params);
    module.CSSeqForm(p_status);
    npivots = module.NumPivots();
    time = module.Time();
    module.GetSolutions(solutions);
    return 1;
  }
  return 1;
}    

void efgLpSolve::SolveSubgame(const FullEfg &/*E*/, const EFSupport &sup,
			      gList<BehavSolution> &solns, gStatus &p_status)
{
  int npiv;
  double time;
  _CSSeqForm(sup, params, solns, p_status, npiv, time);
  npivots += npiv;
}

efgLpSolve::efgLpSolve(const CSSeqFormParams &p, int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

efgLpSolve::~efgLpSolve()   { }

template class CSSeqFormModule<double>;
template class CSSeqFormModule<gRational>;








