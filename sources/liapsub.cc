//
// FILE: liapsub.cc -- Solve efg by liap on nfg
//
// $Id$
//

#include "liapsub.h"

void efgLiapNfgSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				   gList<BehavSolution> &solns)
{
  BehavProfile<double> bp(sup);
  
  subgame_number++;

  gArray<int> infosets(infoset_subgames.Lengths());

  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    int niset = 1;
    for (int iset = 1; iset <= infosets[pl]; iset++)  {
      if (infoset_subgames(pl, iset) == subgame_number)  {
	for (int act = 1; act <= bp.Support().NumActions(pl, niset); act++)
	  bp(pl, niset, act) = start(pl, iset, act);
	niset++;
      }
    }
  }

  Nfg *N = MakeReducedNfg(sup);
  NFSupport support(*N);

  ViewNormal(*N, support);

  MixedProfile<double> mp(support);

  BehavToMixed(E, bp, *N, mp);

  MixedProfile<gNumber> st(support);
  for (int i = 1; i <= st.Length(); i++)
    st[i] = mp[i];

  long this_nevals, this_niters;

  gList<MixedSolution> subsolns;
  Liap(*N, params, st, subsolns, this_nevals, this_niters);

  nevals += this_nevals;

  for (int i = 1; i <= subsolns.Length(); i++)  {
    BehavProfile<gNumber> bp(start);
    MixedToBehav(*N, MixedProfile<gNumber>(subsolns[i]), E, bp);
    solns.Append(bp);
  }

  delete N;
}

efgLiapNfgSolve::efgLiapNfgSolve(const Efg &E, const NFLiapParams &p,
				 const BehavProfile<gNumber> &s, int max)
  : SubgameSolver(max),
    nevals(0), subgame_number(0),
    infoset_subgames(E.NumInfosets()), params(p), start(s)
{ }

efgLiapNfgSolve::~efgLiapNfgSolve()   { }
