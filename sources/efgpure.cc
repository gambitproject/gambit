//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.h"

#include "glist.h"
#include "efg.h"
#include "efgiter.h"
#include "efgciter.h"
#include "behavsol.h"

static int FindPureNash(const EFSupport &p_support,
			gList<BehavSolution> &p_solutions)
{
  int index;
  EfgContIter citer(p_support);
  gPVector<gNumber> probs(p_support.Game().NumInfosets());

  do  {
    int flag = 1;
    citer.GetProfile().InfosetProbs(probs);

    EfgIter eiter(citer);

    for (int pl = 1; flag && pl <= p_support.Game().NumPlayers(); pl++)  {
      gNumber current = citer.Payoff(pl);
      for (int iset = 1; flag && iset <= p_support.Game().Players()[pl]->NumInfosets();
	         iset++)  {
      	if (probs(pl, iset) == gNumber(0))   continue;
	      Infoset *s = p_support.Game().Players()[pl]->Infosets()[iset];
       	for (int act = 1; act <= s->NumActions(); act++)  {
	  eiter.Next(pl, iset);
	  if (eiter.Payoff(pl) > current)  {
	    flag = 0;
	    break;
	  }
      	}
      }
    }

    if (flag)  {
      BehavProfile<gNumber> temp(p_support);
      // zero out all the entries, since any equilibria are pure
      ((gVector<gNumber> &) temp).operator=(gNumber(0));
      const PureBehavProfile<gNumber> &profile = citer.GetProfile();
      for (int pl = 1; pl <= p_support.Game().NumPlayers(); pl++)  {
	for (int iset = 1; iset <= p_support.Game().Players()[pl]->NumInfosets();
	     iset++)
	  temp(pl, iset, profile.GetAction(p_support.Game().Players()[pl]->Infosets()[iset])->GetNumber()) = 1;
      }

      index = p_solutions.Append(BehavSolution(temp, EfgAlg_PURENASH));
      p_solutions[index].SetIsNash(triTRUE);
    }
  }  while (citer.NextContingency());

  return p_solutions.Length();
}

int efgEnumPure::SolveSubgame(const Efg &/*E*/, const EFSupport &sup,
			      gList<BehavSolution> &solns)
{
  FindPureNash(sup, solns);

  return 0;
}

efgEnumPure::efgEnumPure(int max)
  : SubgameSolver(max)
{ }

efgEnumPure::~efgEnumPure()   { }

