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

  Enum(*S, params, values, solutions, npiv, time);

  npivots += npiv;
  
  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup, values);
    MixedToBehav(*N, solutions[i], E, bp);
    solns.Append(bp);
  }

  delete S;
  delete N;

  return params.status.Get();
}

EnumBySubgame::EnumBySubgame(const EFSupport &S, const gArray<gNumber> &v,
			     const EnumParams &p, int max)
  : SubgameSolver(S, v, max), npivots(0), params(p), values(v)
{ }

EnumBySubgame::~EnumBySubgame()   { }

int Enum(const EFSupport &support, const EnumParams &params,
	 const gArray<gNumber> &values, 
	 gList<BehavSolution> &solutions, long &npivots, double &time)
{
  EnumBySubgame module(support, values, params);
  module.Solve();
  npivots = module.NumPivots();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

