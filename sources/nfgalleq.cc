//
// FILE: nfgalleq.cc -- All Nash Enum module
//
// $Id$
//

#include "nfgalleq.imp"

int NfgAllNash(const Nfg &N, const PolEnumParams &params,
	       gList<MixedSolution> &solutions, long &nevals, double &time)
{
  if (params.precision == precDOUBLE)  {
    NfgAllNashModule<gDouble> module(N, params);
    module.NashEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    NfgAllNashModule<gRational> module(N, params);
    module.NashEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

template class NfgAllNashModule<double>;
template class NfgAllNashModule<gDouble>;
template class NfgAllNashModule<gRational>;











