//
// FILE: simpsub.cc -- Instantiation of SimpdivBySubgame
//
// $Id$
//

#include "simpsub.h"

int SimpdivBySubgame::SolveSubgame(const Efg &E, const EFSupport &sup,
				   gList<BehavSolution> &solns)
{
  Nfg *N = MakeReducedNfg(E, sup);

  NFSupport *S = new NFSupport(*N);

  ViewNormal(*N, S);

  gList<MixedSolution> solutions;
  int nev;
  int nit;
  double time;
  Simpdiv(*S, params, values, solutions, nev, nit, time);

  nevals += nev;

  for (int i = 1; i <= solutions.Length(); i++)  {
    BehavProfile<gNumber> bp(sup, values);
    MixedToBehav(*N, solutions[i], E, bp);
    solns.Append(bp);
  }

  delete S;
  delete N;
  return params.status.Get();
}

SimpdivBySubgame::SimpdivBySubgame(const EFSupport &S, 
				   const gArray<gNumber> &v,
				   const SimpdivParams &p, int max)
  : SubgameSolver(S, v, max), params(p), values(v)
{ }

SimpdivBySubgame::~SimpdivBySubgame()   { }

int Simpdiv(const EFSupport &support, const SimpdivParams &params,
	    const gArray<gNumber> &values, gList<BehavSolution> &solutions,
	    int &nevals, int &/*niters*/, double &time)
{
  SimpdivBySubgame module(support, values, params);
  module.Solve();
  
  solutions = module.GetSolutions();
  nevals = module.NumEvals();
  time = module.Time();
  return 1;
}
