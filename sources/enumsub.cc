//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "enumsub.h"

void EnumBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
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
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete N;
}

EnumBySubgame::EnumBySubgame(const EFSupport &S, const EnumParams &p, int max)
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

