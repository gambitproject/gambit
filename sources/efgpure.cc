//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.imp"
#include "rational.h"

int EnumPure(const EFSupport &support, const gArray<gNumber> &values,
	     gList<BehavSolution> &solutions, double &time)
{
  EfgPSNEBySubgame module(support, values);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

int EfgPSNEBySubgame::SolveSubgame(const Efg &/*E*/, const EFSupport &sup,
				   gList<BehavSolution> &solns)
{
  FindPureNash(sup, values, solns);

  return 0;
}

EfgPSNEBySubgame::EfgPSNEBySubgame(const EFSupport &S, 
				   const gArray<gNumber> &v, int max)
  : SubgameSolver(S, v, max), values(v)
{ }

EfgPSNEBySubgame::~EfgPSNEBySubgame()   { }

