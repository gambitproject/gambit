//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.imp"
#include "rational.h"

int EnumPure(const EFSupport &support,
	     gList<BehavSolution> &solutions, double &time)
{
  EfgPSNEBySubgame module(support);
  module.Solve();
  time = module.Time();
  solutions = module.GetSolutions();
  return 1;
}

int EfgPSNEBySubgame::SolveSubgame(const Efg &/*E*/, const EFSupport &sup,
				   gList<BehavSolution> &solns)
{
  FindPureNash(sup, solns);

  return 0;
}

EfgPSNEBySubgame::EfgPSNEBySubgame(const EFSupport &S, int max)
  : SubgameSolver(S, max)
{ }

EfgPSNEBySubgame::~EfgPSNEBySubgame()   { }

