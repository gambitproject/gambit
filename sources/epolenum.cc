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

BehavSolution PolishEquilibrium(const EFSupport &support, 
				const BehavSolution &sol, 
				bool &is_singular)
{
  EfgPolEnumParams params;
  EfgPolEnumModule<gDouble> module(support, params);
  gVector<gDouble> vec = module.SolVarsFromBehavProfile(*(sol.Profile()));

  /* //DEBUG
  gPVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  BehavProfile<gNumber> newsol = module.SequenceForm().ToBehav(xx);

  gout << "sol.Profile = " << *(sol.Profile()) << "\n";
  gout << "vec  = " << vec << "\n";
  gout << "xx   = " << xx << "\n";
  gout << "newsol   = " << newsol << "\n";

    exit(0);
  if ( newsol != *(sol.Profile()) ) {
    gout << "Failure of reversibility in PolishEquilibrium.\n";
    exit(0);
  }
  */

  //DEBUG
  //  gout << "Prior to Polishing vec is " << vec << ".\n";

  module.PolishKnownRoot(vec);

  //DEBUG
  //  gout << "After Polishing vec is " << vec << ".\n";

  return module.ReturnPolishedSolution(vec);
}

