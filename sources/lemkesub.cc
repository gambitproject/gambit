//
// FILE: lemkesub.cc -- Instantiation of LemkeBySubgame
//
// $Id$
//

#include "lemkesub.h"

void efgLcpNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
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
    MixedProfile<gNumber> profile(solutions[i]);
    solns.Append(BehavProfile<gNumber>(profile));
  }

  delete N;
}

efgLcpNfgSolve::efgLcpNfgSolve(const EFSupport &, const LemkeParams &p,
			       int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

efgLcpNfgSolve::~efgLcpNfgSolve()   { }

