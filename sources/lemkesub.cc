//
// FILE: lemkesub.cc -- Instantiation of LemkeBySubgame
//
// $Id$
//

#include "lemkesub.h"

void efgLcpNfgSolve::SolveSubgame(const Efg &E, const EFSupport &sup,
				  gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);

  gList<MixedSolution> solutions;

  int npiv;
  double time;

  Lemke(support, params, solutions, npiv, time);

  npivots += npiv;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete N;
}

efgLcpNfgSolve::efgLcpNfgSolve(const EFSupport &S, const LemkeParams &p,
			       int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

efgLcpNfgSolve::~efgLcpNfgSolve()   { }

