//
// FILE: simpsub.cc -- Instantiation of SimpdivBySubgame
//
// $Id$
//

#include "simpsub.h"

int SimpdivBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
				   gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(sup);

  NFSupport *S = new NFSupport(*N);

  ViewNormal(*N, S);

  gList<MixedSolution> solutions;
  int nev;
  int nit;
  double time;
  Simpdiv(*S, params, solutions, nev, nit, time);

  nevals += nev;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup);
    MixedToBehav(*N, MixedProfile<gNumber>(solutions[i]), E, bp);
    solns.Append(bp);
  }

  delete S;
  delete N;
  return 0;
}

SimpdivBySubgame::SimpdivBySubgame(const EFSupport &S, 
				   const SimpdivParams &p, int max)
  : SubgameSolver(S, max), params(p)
{ }

SimpdivBySubgame::~SimpdivBySubgame()   { }

int Simpdiv(const EFSupport &support, const SimpdivParams &params,
	    gList<BehavSolution> &solutions,
	    int &nevals, int &/*niters*/, double &time)
{
  SimpdivBySubgame module(support, params);
  module.Solve();
  
  solutions = module.GetSolutions();
  nevals = module.NumEvals();
  time = module.Time();
  return 1;
}
