//
// FILE: psnesub.cc -- Enumerating of pure mixed strategies
//
// $Id$
//

#include "psnesub.h"
#include "nfgpure.h"

void efgEnumPureNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				       gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);

  gList<MixedSolution> solutions;
  nfgEnumPure solver;
  solver.SetStopAfter(m_stopAfter);
  solver.Solve(support, m_status, solutions);

  for (int i = 1; i <= solutions.Length(); i++)  {
    MixedProfile<gNumber> profile(solutions[i]);
    solns.Append(BehavProfile<gNumber>(profile));
  }

  delete N;
}

efgEnumPureNfgSolve::efgEnumPureNfgSolve(const EFSupport &,
					 int p_stopAfter, gStatus &p_status)
  : SubgameSolver(0), m_stopAfter(p_stopAfter), m_status(p_status)
{ }

efgEnumPureNfgSolve::~efgEnumPureNfgSolve()   { }


