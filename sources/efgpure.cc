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

static void efgEnumPureSolve(const EFSupport &p_support,
			     gList<BehavSolution> &p_solutions,
			     int p_stopAfter, gStatus &p_status)
{
  EfgContIter citer(p_support);
  gPVector<gNumber> probs(p_support.Game().NumInfosets());

  int ncont = 1;
  for (int pl = 1; pl <= p_support.Game().NumPlayers(); pl++) {
    EFPlayer *player = p_support.Game().Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      ncont *= p_support.NumActions(pl, iset);
  }

  int contNumber = 1;
  do  {
    p_status.Get();
    p_status.SetProgress((double) contNumber / (double) ncont);

    bool flag = true;
    citer.GetProfile().InfosetProbs(probs);

    EfgIter eiter(citer);

    for (int pl = 1; flag && pl <= p_support.Game().NumPlayers(); pl++)  {
      gNumber current = citer.Payoff(pl);
      for (int iset = 1;
	   flag && iset <= p_support.Game().Players()[pl]->NumInfosets();
	   iset++)  {
      	if (probs(pl, iset) == gNumber(0))   continue;
       	for (int act = 1; act <= p_support.NumActions(pl, iset); act++)  {
	  eiter.Next(pl, iset);
	  if (eiter.Payoff(pl) > current)  {
	    flag = false;
	    break;
	  }
      	}
      }
    }

    if (flag)  {
      BehavProfile<gNumber> temp(EFSupport(p_support.Game()));
      // zero out all the entries, since any equilibria are pure
      ((gVector<gNumber> &) temp).operator=(gNumber(0));
      const PureBehavProfile<gNumber> &profile = citer.GetProfile();
      for (int pl = 1; pl <= p_support.Game().NumPlayers(); pl++)  {
	for (int iset = 1;
	     iset <= p_support.Game().Players()[pl]->NumInfosets();
	     iset++)
	  temp(pl, iset,
	       profile.GetAction(p_support.Game().Players()[pl]->
				 Infosets()[iset])->GetNumber()) = 1;
      }

      p_solutions.Append(BehavSolution(temp, algorithmEfg_ENUMPURE_EFG));
    }
    contNumber++;
  }  while ((p_stopAfter == 0 || p_solutions.Length() < p_stopAfter) &&
	    citer.NextContingency());
}

void efgEnumPure::SolveSubgame(const Efg &, const EFSupport &p_support,
			       gList<BehavSolution> &p_solutions)
{
  efgEnumPureSolve(p_support, p_solutions, m_stopAfter, m_status);
}

efgEnumPure::efgEnumPure(int p_stopAfter, gStatus &p_status)
  : SubgameSolver(0), m_stopAfter(p_stopAfter), m_status(p_status)
{ }

efgEnumPure::~efgEnumPure()   { }





