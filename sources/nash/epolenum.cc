//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via solving polynomial equations
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "epolenum.imp"

//---------------------------------------------------------------------------
//                        EfgPolEnumParams: member functions
//---------------------------------------------------------------------------

EfgPolEnumParams::EfgPolEnumParams(void)
  : stopAfter(0)
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

