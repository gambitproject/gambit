//
// FILE: simpsub.cc -- Instantiation of SimpdivBySubgame
//
// $Id$
//

#include "simpsub.h"

void efgSimpDivNfgSolve::SolveSubgame(const Efg &E, const EFSupport &sup,
				      gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);

  gList<MixedSolution> solutions;
  int nev, nit;
  double time;
  Simpdiv(support, params, solutions, nev, nit, time);

  nevals += nev;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete N;
}

efgSimpDivNfgSolve::efgSimpDivNfgSolve(const EFSupport &, 
				       const SimpdivParams &p, int max)
  : SubgameSolver(max), params(p)
{ }

efgSimpDivNfgSolve::~efgSimpDivNfgSolve()   { }
