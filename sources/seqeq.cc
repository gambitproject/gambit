//
// FILE: seqeq.cc  Sequential equilibrium solver
//
// $Id$ 
//

#include "seqeq.imp"
//---------------------------------------------------------------------------
//                        SeqEquilibParams: member functions
//---------------------------------------------------------------------------

SeqEquilibParams::SeqEquilibParams(gStatus &s)
  : AlgParams(s)
{ }

int SequentialEquilib(const EFBasis &b, const EFSupport &B, 
		      const SeqEquilibParams &params,
		      gList<BehavSolution> & solutions, 
		      long &nevals, double &time)
{
  SequentialEquilibModule<gDouble> module(b,B, params);
  module.SequentialEquilib();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

#include "double.h"

template class SequentialEquilibModule<gDouble>;


