//
// FILE: polensub.cc -- Implementation of PolEnumBySubgame
//
// $Id$ 
//

#include "polensub.h"

void efgPolEnumNfgSolve::SolveSubgame(const Efg &E, const EFSupport &sup,
				      gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);

  NFSupport *S = new NFSupport(*N);
  
  ViewNormal(*N, S);
  
  gList<MixedSolution> solutions;

  long neval;
  double time;

  try {
    PolEnum(*S, params, solutions, neval, time);

    nevals += neval;
  
    for (int i = 1; i <= solutions.Length(); i++)  {
      BehavProfile<gNumber> bp(sup);
      MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
      solns.Append(bp);
    }

    delete S;
    delete N;
  }
  catch (...) {
    delete S;
    delete N;
    throw;
  }
}

efgPolEnumNfgSolve::efgPolEnumNfgSolve(const EFSupport &S,
				       const PolEnumParams &p, int max)
  : SubgameSolver(max), nevals(0), params(p)
{ }

efgPolEnumNfgSolve::~efgPolEnumNfgSolve()   { }

