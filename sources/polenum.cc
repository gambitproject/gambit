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
	    gList<MixedSolution> &solutions, long &nevals, double &time,
	    bool &is_singular)
{
  if (params.precision == precDOUBLE)  {
    PolEnumModule<gDouble> module(support, params);
    module.PolEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
    if (module.IsSingular()) 
      is_singular = true;
    else 
      is_singular = false;
  }
  else if (params.precision == precRATIONAL)  {
    PolEnumModule<gRational> module(support, params);
    module.PolEnum();
    nevals = module.NumEvals();
    time = module.Time();
    solutions = module.GetSolutions();
    if (module.IsSingular()) 
      is_singular = true;
    else 
      is_singular = false;
  }

  return 1;
}

#include "rational.h"
#include "double.h"

gPoly<gDouble> 
PolEnumModule<gDouble>::Equation(int i, int strat1, int strat2) const
{
  StrategyProfile profile(NF);

  NfgContIter A(support), B(support);
  A.Freeze(i);
  A.Set(i, strat1);
  B.Freeze(i);
  B.Set(i, strat2);
  gPoly<gDouble> equation(&Space,&Lex);
  do {
    gPoly<gDouble> term(&Space,(gDouble)1,&Lex);
    profile = A.Profile();
    int k;
    for(k=1;k<=NF.NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k,support.Find(profile[k]));
    gDouble coeff,ap,bp;
    ap = (double)NF.Payoff(A.GetOutcome(), i);
    bp = (double)NF.Payoff(B.GetOutcome(), i);
    coeff = ap - bp;
    term*=coeff;
    equation+=term;
    A.NextContingency();
  } while (B.NextContingency());
  return equation;
}

template class PolEnumModule<double>;
template class PolEnumModule<gDouble>;
template class PolEnumModule<gRational>;











