//
// FILE: polensub.cc -- Implementation of PolEnumBySubgame
//
// $Id$ 
//

#include "polensub.h"

void efgPolEnumNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				      gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);
  
  gList<MixedSolution> solutions;

  long neval;
  double time;

  try {
    bool is_singular = false; // This info is currently unutilized!!
    PolEnum(support, params, solutions, neval, time, is_singular);

    nevals += neval;
  
    for (int i = 1; i <= solutions.Length(); i++)  {
      BehavProfile<gNumber> bp(sup);
      MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
      solns.Append(bp);
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

