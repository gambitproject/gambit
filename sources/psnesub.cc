//
// FILE: psnesub.cc -- Enumerating of pure mixed strategies
//
// $Id$
//

#include "psnesub.h"
#include "nfgpure.h"

void efgEnumPureNfgSolve::SolveSubgame(const Efg &E, const EFSupport &sup,
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
}

efgEnumPureNfgSolve::efgEnumPureNfgSolve(const EFSupport &p_support,
					 gStatus &p_status, int p_max) 
  : SubgameSolver(p_max), m_status(p_status)
{ }

efgEnumPureNfgSolve::~efgEnumPureNfgSolve()   { }
