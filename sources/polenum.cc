//
// FILE: polenum.cc -- Polynomial Nash Enum module
//
// $Id$
//

#include "polenum.imp"

//---------------------------------------------------------------------------
//                        PolEnumParams: member functions
//---------------------------------------------------------------------------

PolEnumParams::PolEnumParams(gStatus &status_)
  : trace(0), stopAfter(0), precision(precDOUBLE),
    tracefile(&gnull), status(status_)
{ }

int PolEnum(const NFSupport &support, const PolEnumParams &params,
	 const gArray<gNumber> &values,
	 gList<MixedSolution> &solutions, long &nevals, double &time)
{
  if (params.precision == precDOUBLE)  {
    PolEnumModule<gDouble> module(support, params, values);
    module.PolEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    PolEnumModule<gRational> module(support, params, values);
    module.PolEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
  }

  return 1;
}

#include "rational.h"
#include "double.h"

template class PolEnumModule<double>;
template class PolEnumModule<gDouble>;
template class PolEnumModule<gRational>;











