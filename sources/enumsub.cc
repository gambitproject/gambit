//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "enumsub.h"

void EnumBySubgame::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				 gList<BehavSolution> &solns,
				 gStatus &p_status)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);
  
  gList<MixedSolution> solutions;

  long npiv;
  double time;

  Enum(support, params, solutions, gnull, p_status, npiv, time);

  npivots += npiv;
  
  for (int i = 1; i <= solutions.Length(); i++)  {
    MixedProfile<gNumber> profile(solutions[i]);
    solns.Append(BehavProfile<gNumber>(profile));
  }

  delete N;
}

EnumBySubgame::EnumBySubgame(const EnumParams &p, int max)
  : SubgameSolver(max), npivots(0), params(p)
{ }

EnumBySubgame::~EnumBySubgame()   { }

int Enum(const EFSupport &support, const EnumParams &params,
	 gList<BehavSolution> &solutions, gStatus &p_status,
	 long &npivots, double &time)
{
  EnumBySubgame module(params);
  solutions = module.Solve(support, p_status);
  npivots = module.NumPivots();
  time = module.Time();
  return 1;
}

