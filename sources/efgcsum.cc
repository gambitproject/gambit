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

CSSeqFormParams::CSSeqFormParams(gStatus &s) 
  :  AlgParams(s)
{ }


//-----------------------------------
// Interfacing to solve-by-subgame
//-----------------------------------

static int _CSSeqForm(const EFSupport &support, const CSSeqFormParams &params,
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

void efgLpSolve::SolveSubgame(const Efg &/*E*/, const EFSupport &sup,
			      gList<BehavSolution> &solns)
{
  int npiv;
  double time;
  _CSSeqForm(sup, params, solns, npiv, time);
  npivots += npiv;
}

efgLpSolve::efgLpSolve(const EFSupport &S, const CSSeqFormParams &p, int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

efgLpSolve::~efgLpSolve()   { }

template class CSSeqFormModule<double>;
template class CSSeqFormModule<gRational>;








