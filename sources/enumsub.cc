//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "enumsub.h"

int EnumBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
				gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(E, sup);

  NFSupport *S = new NFSupport(*N);
  
  ViewNormal(*N, S);
  
  gList<MixedSolution> solutions;

  long npiv;
  double time;

  Enum(*S, params, solutions, npiv, time);

  npivots += npiv;
  
  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete S;
  delete N;

  return params.status.Get();
}

EnumBySubgame::EnumBySubgame(const EFSupport &S, const EnumParams &p, int max)
  : SubgameSolver(S, max), npivots(0), params(p)
{ }

EnumBySubgame::~EnumBySubgame()   { }

int Enum(const EFSupport &support, const EnumParams &params,
	 gList<BehavSolution> &solutions, long &npivots, double &time)
{
  EnumBySubgame module(support, params);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

