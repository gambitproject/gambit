//
// FILE: epolenum.cc  Extensive form version of polynomial enumeration
//
// $Id$
//

#include "epolenum.imp"
//---------------------------------------------------------------------------
//                        EfgPolEnumParams: member functions
//---------------------------------------------------------------------------

EfgPolEnumParams::EfgPolEnumParams(gStatus &status_)
  : trace(0), stopAfter(0), precision(precDOUBLE),
    tracefile(&gnull), status(status_)
{ }

int EfgPolEnum(const EFSupport &support, const EfgPolEnumParams &params,
	 const gArray<gNumber> &values,
	 gList<BehavSolution> & /* solutions*/, long &nevals, double &time)
{
  if (params.precision == precDOUBLE)  {
    EfgPolEnumModule<gDouble> module(support, params, values);
    module.EfgPolEnum();
    nevals = module.NumEvals();
    time = module.Time();
//    solutions = module.GetSolutions();
  }
  else if (params.precision == precRATIONAL)  {
    EfgPolEnumModule<gRational> module(support, params, values);
    module.EfgPolEnum();
    nevals = module.NumEvals();
    time = module.Time();
//    solutions = module.GetSolutions();
  }

  return 1;
}

#include "rational.h"
#include "double.h"

template class EfgPolEnumModule<double>;
template class EfgPolEnumModule<gDouble>;
template class EfgPolEnumModule<gRational>;

