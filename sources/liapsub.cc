//
// FILE: liapsub.cc -- Solve efg by liap on nfg
//
// $Id$
//

#include "liapsub.h"

int NFLiapBySubgame::SolveSubgame(const Efg<double> &E,
				   gList<BehavSolution<double> > &solns)
{
  Nfg<double> *N = MakeReducedNfg((Efg<double> &) E);

  NFSupport *S=new NFSupport(*N);

  ViewNormal(*N, S);

  MixedProfile<double> mp(*N, *S);
  
  long this_nevals, this_niters;

  gList<MixedSolution<double> > subsolns;
  Liap(*N, params, mp, subsolns, this_nevals, this_niters);

  nevals += this_nevals;

  for (int i = 1; i <= subsolns.Length(); i++)  {
    BehavProfile<double> bp(E);
    MixedToBehav(*N, subsolns[i], E, bp);
    solns.Append(bp);
  }

  delete N;
  return params.status.Get();
}

NFLiapBySubgame::NFLiapBySubgame(const Efg<double> &E, const NFLiapParams &p,
				 const BehavProfile<double> &s, int max)
  : SubgameSolver<double>(E, max), nevals(0), params(p), start(s)
{ }

NFLiapBySubgame::~NFLiapBySubgame()   { }
