//
// FILE: polensub.cc -- Implementation of PolEnumBySubgame
//
// $Id$ 
//

#include "polensub.h"
#include "nfgalleq.h"

void efgPolEnumNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				      gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);
  
  gList<MixedSolution> solutions;

  long neval = 0;
  double time = 0.0;
  gList<const NFSupport> singular_supports;

  try {
    AllNashSolve(support, params, solutions, neval, time, singular_supports);

    nevals += neval;
  
    for (int i = 1; i <= solutions.Length(); i++)  {
      MixedProfile<gNumber> profile(solutions[i]);
      solns.Append(BehavProfile<gNumber>(profile));
    }
    delete N;
  }
  catch (...) {
    delete N;
    throw;
  }
}

efgPolEnumNfgSolve::efgPolEnumNfgSolve(const EFSupport &,
				       const PolEnumParams &p, int max)
  : SubgameSolver(max), nevals(0), params(p)
{ }

efgPolEnumNfgSolve::~efgPolEnumNfgSolve()   { }

