//
// FILE: epolenum.cc  Extensive form version of polynomial enumeration
//
// $Id$
//

#include "epolenum.imp"
//---------------------------------------------------------------------------
//                        EfgPolEnumParams: member functions
//---------------------------------------------------------------------------

EfgPolEnumParams::EfgPolEnumParams(gStatus &s)
  : AlgParams(s)
{ }

int EfgPolEnum(const EFSupport &support, const EfgPolEnumParams &params,
	       gList<BehavSolution> & solutions, long &nevals, double &time,
	       bool &is_singular)
{
  EfgPolEnumModule<gDouble> module(support, params);
  module.EfgPolEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  is_singular = module.IsSingular();

  return 1;
}

#include "double.h"

gPoly<gDouble> 
EfgPolEnumModule<gDouble>::Payoff(int pl) const
{
  gIndexOdometer index(SF.NumSequences());
  gNumber pay;

  gPoly<gDouble> equation(Space,Lex);
  while (index.Turn()) {
    pay=SF.Payoff(index.CurrentIndices(),pl);
    if( pay !=(gNumber)0) {
      gPoly<gDouble> term(Space,(gDouble)((double)pay),Lex);
      int k;
      for(k=1;k<=EF.NumPlayers();k++) 
	term*=Prob(k,(index.CurrentIndices())[k]);
      equation+=term;
    }
  }
  if(params.trace >1)
    (*params.tracefile) << "\nPayoff( " << pl << "): " << equation;
  return equation;
}

template class EfgPolEnumModule<gDouble>;
