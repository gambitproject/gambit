//
// FILE: psnesub.cc -- Instantiation of PureNashBySubgame 
//
// $Id$
//

#include "psnesub.h"
#include "nfgpure.h"

int PureNashBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
				    gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);

  NFSupport *S = new NFSupport(*N);

  ViewNormal(*N, S);

  gList<MixedSolution> sol;
  FindPureNash(*N, *S, sol);

  for (int i = 1; i <= sol.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(MixedProfile<gNumber>(sol[i]), bp);
    solns.Append(bp);
  }

  delete S;
  delete N;
  // return params.status.Get();
  return 0;
}

PureNashBySubgame::PureNashBySubgame(const EFSupport &S, int max) 
  : SubgameSolver(S, max)
{ }

PureNashBySubgame::~PureNashBySubgame()   { }

int EnumPureNfg(const EFSupport &support, 
		gList<BehavSolution> &solutions, double &time)
{
  PureNashBySubgame module(support);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}
