//
// FILE: lemke.cc -- Lemke-Howson module
//
// $Id$
//

#include "lemke.imp"

//---------------------------------------------------------------------------
//                       LemkeParams: member functions
//---------------------------------------------------------------------------

LemkeParams::LemkeParams(gStatus &s) 
  : dup_strat(0), trace(0), stopAfter(0), tracefile(&gnull), status(s)
{ }



int Lemke(const NFSupport &S, const LemkeParams &LP,
          gList<MixedSolution> &solutions,
          int &npivots, double &time)
{
  if (LP.precision == precDOUBLE)  {
    LemkeModule<double> LS(S.Game(), LP, S);
    LS.Lemke();
    for (int i = 1; i <= LS.GetSolutions().Length(); i++)  
      solutions.Append(MixedSolution(LS.GetSolutions()[i]));
    npivots = LS.NumPivots();
    time = LS.Time();
  }
  else  {
    LemkeModule<gRational> LS(S.Game(), LP, S);
    LS.Lemke();
    for (int i = 1; i <= LS.GetSolutions().Length(); i++)  
      solutions.Append(MixedSolution(LS.GetSolutions()[i]));
    solutions = LS.GetSolutions();
    npivots = LS.NumPivots();
    time = LS.Time();
  }

  return 1;
}

#include "rational.h"

template class LemkeModule<double>;
template class LemkeModule<gRational>;


