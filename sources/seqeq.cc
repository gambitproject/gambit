//
// FILE: seqeq.cc  Sequential equilibrium solver
//
// $Id$ 
//

#include "seqeq.imp"
//---------------------------------------------------------------------------
//                        SeqEquilibParams: member functions
//---------------------------------------------------------------------------

SeqEquilibParams::SeqEquilibParams(gStatus &status_)
  : trace(0), stopAfter(0), precision(precDOUBLE),
    tracefile(&gnull), status(status_)
{ }

int SequentialEquilib(const EFBasis &b, const EFSupport &B, 
		      const SeqEquilibParams &params,
		      gList<BehavSolution> & solutions, 
		      long &nevals, double &time)
{
  if (params.precision == precDOUBLE)  {
    SequentialEquilibModule<gDouble> module(b,B, params);
    module.SequentialEquilib();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    SequentialEquilibModule<gRational> module(b,B, params);
    module.SequentialEquilib();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

#include "rational.h"
#include "double.h"

template class SequentialEquilibModule<double>;
template class SequentialEquilibModule<gDouble>;
template class SequentialEquilibModule<gRational>;


