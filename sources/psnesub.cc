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
  FindPureNash(*S, 0, m_status, sol);

  for (int i = 1; i <= sol.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(sol[i]), E, bp);
    solns.Append(bp);
  }

  delete S;
  delete N;
  return 0;
}

PureNashBySubgame::PureNashBySubgame(const EFSupport &p_support,
				     gStatus &p_status, int p_max) 
  : SubgameSolver(p_max), m_status(p_status)
{ }

PureNashBySubgame::~PureNashBySubgame()   { }

int EnumPureNfg(const EFSupport &p_support, int p_max, gStatus &p_status,
		gList<BehavSolution> &p_solutions, double &p_time)
{
  PureNashBySubgame module(p_support, p_status, p_max);
  module.Solve(p_support);
  p_time = module.Time();
  p_solutions = module.GetSolutions();
  return 1;
}
