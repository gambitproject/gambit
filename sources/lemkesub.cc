//
// FILE: lemkesub.cc -- Instantiation of LemkeBySubgame
//
// $Id$
//

#include "lemkesub.h"

int LemkeBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
				 gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);

  NFSupport *S = new NFSupport(*N);

  ViewNormal(*N, S);

  gList<MixedSolution> solutions;

  int npiv;
  double time;

  Lemke(*S, params, solutions, npiv, time);

  npivots += npiv;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(MixedProfile<gNumber>(solutions[i]), bp);
    solns.Append(bp);
  }

  delete S;
  delete N;
  return 0;
}

LemkeBySubgame::LemkeBySubgame(const EFSupport &S, const LemkeParams &p,
			       int max)
  : SubgameSolver(S, max), npivots(0), params(p)
{ }

LemkeBySubgame::~LemkeBySubgame()   { }


int Lemke(const EFSupport &support, const LemkeParams &params,
	  gList<BehavSolution> &solutions, int &npivots, double &time)
{
  LemkeBySubgame module(support, params);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}
