//
// FILE: epolenum.cc  Extensive form version of polynomial enumeration
//
// $Id$
//

#include "epolenum.imp"

//---------------------------------------------------------------------------
//                        EfgPolEnumParams: member functions
//---------------------------------------------------------------------------

EfgPolEnumParams::EfgPolEnumParams(void)
{ }

//---------------------------------------------------------------------------
//                    EfgPolEnum: nontemplate functions
//---------------------------------------------------------------------------

template class EfgPolEnumModule<gDouble>;

int EfgPolEnum(const EFSupport &support, const EfgPolEnumParams &params,
	       gList<BehavSolution> &solutions, gStatus &p_status,
	       long &nevals, double &time, bool &is_singular)
{
  EfgPolEnumModule<gDouble> module(support, params);
  module.EfgPolEnum(p_status);
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  is_singular = module.IsSingular();

  return 1;
}

