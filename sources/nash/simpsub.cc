//
// FILE: simpsub.cc -- Instantiation of SimpdivBySubgame
//
// $Id$
//

#include "simpsub.h"

void efgSimpDivNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				      gList<BehavSolution> &solns,
				      gStatus &p_status)
{
  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);
  ViewNormal(*N, support);

  gList<MixedSolution> solutions;
  int nev, nit;
  double time;
  Simpdiv(support, params, solutions, p_status, nev, nit, time);

  nevals += nev;

  for (int i = 1; i <= solutions.Length(); i++)  {
    MixedProfile<gNumber> profile(*solutions[i].Profile());
    solns.Append(BehavProfile<gNumber>(profile));
  }

  delete N;
}

efgSimpDivNfgSolve::efgSimpDivNfgSolve(const SimpdivParams &p, int max)
  : SubgameSolver(max), params(p)
{ }

efgSimpDivNfgSolve::~efgSimpDivNfgSolve()   { }
