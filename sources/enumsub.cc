//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "enumsub.h"

void EnumBySubgame::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				 gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);
  
  gList<MixedSolution> solutions;

  long npiv;
  double time;

  Enum(support, params, solutions, npiv, time);

  npivots += npiv;
  
  for (int i = 1; i <= solutions.Length(); i++)  {
    MixedProfile<gNumber> profile(solutions[i]);
    solns.Append(BehavProfile<gNumber>(profile));
  }

  delete N;
}

EnumBySubgame::EnumBySubgame(const EFSupport &, const EnumParams &p, int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

EnumBySubgame::~EnumBySubgame()   { }

int Enum(const EFSupport &support, const EnumParams &params,
	 gList<BehavSolution> &solutions, long &npivots, double &time)
{
  EnumBySubgame module(support, params);
  solutions = module.Solve(support);
  npivots = module.NumPivots();
  time = module.Time();
  return 1;
}

