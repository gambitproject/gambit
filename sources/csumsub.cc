//
// FILE: csumsub.cc -- Instantiation of ZSumBySubgame
//
// $Id$
//

#include "csumsub.h"
#include "nfgcsum.h"

int ZSumBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
				gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);

  NFSupport *S = new NFSupport(*N);

  ViewNormal(*N, S);

  int npiv;
  double time;
  gList<MixedSolution> solutions;
  ZSum(*S, params, solutions, npiv, time);

  npivots += npiv;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete S;
  delete N;
  // return params.status.Get();
  return 0;
}

ZSumBySubgame::ZSumBySubgame(const EFSupport &S, const ZSumParams &p,
			     int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

ZSumBySubgame::~ZSumBySubgame()   { }


int ZSum(const EFSupport &support, const ZSumParams &params,
	 gList<BehavSolution> &solutions, int &npivots, double &time)
{
  ZSumBySubgame module(support, params);
  module.Solve(support);
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

