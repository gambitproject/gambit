//
// FILE: csumsub.cc -- Instantiation of ZSumBySubgame
//
// $Id$
//

#include "csumsub.h"
#include "nfgcsum.h"

void efgLpNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				 gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);

  int npiv;
  double time;
  gList<MixedSolution> solutions;
  ZSum(support, params, solutions, npiv, time);

  npivots += npiv;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete N;
}

efgLpNfgSolve::efgLpNfgSolve(const EFSupport &, const ZSumParams &p,
			     int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

efgLpNfgSolve::~efgLpNfgSolve()   { }
