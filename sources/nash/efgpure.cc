//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute pure-strategy Nash equilibria on extensive form games
//

#include "base/base.h"
#include "efgpure.h"

#include "game/efg.h"
#include "game/efgiter.h"
#include "game/efgciter.h"
#include "behavsol.h"

gList<BehavSolution> efgEnumPure::Solve(const EFSupport &p_support,
					gStatus &p_status)
{
  gList<BehavSolution> solutions;

  EfgContIter citer(p_support);
  gPVector<gNumber> probs(p_support.GetGame().NumInfosets());

  int ncont = 1;
  for (int pl = 1; pl <= p_support.GetGame().NumPlayers(); pl++) {
    EFPlayer *player = p_support.GetGame().Players()[pl];
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

    for (int pl = 1; flag && pl <= p_support.GetGame().NumPlayers(); pl++)  {
      gNumber current = citer.Payoff(pl);
      for (int iset = 1;
	   flag && iset <= p_support.GetGame().Players()[pl]->NumInfosets();
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
      BehavProfile<gNumber> temp(EFSupport(p_support.GetGame()));
      // zero out all the entries, since any equilibria are pure
      ((gVector<gNumber> &) temp).operator=(gNumber(0));
      const PureBehavProfile<gNumber> &profile = citer.GetProfile();
      for (int pl = 1; pl <= p_support.GetGame().NumPlayers(); pl++)  {
	for (int iset = 1;
	     iset <= p_support.GetGame().Players()[pl]->NumInfosets();
	     iset++)
	  temp(pl, iset,
	       profile.GetAction(p_support.GetGame().Players()[pl]->
				 Infosets()[iset])->GetNumber()) = 1;
      }

      solutions.Append(BehavSolution(temp, algorithmEfg_ENUMPURE_EFG));
    }
    contNumber++;
  }  while ((m_stopAfter == 0 || solutions.Length() < m_stopAfter) &&
	    citer.NextContingency());
  return solutions;
}






