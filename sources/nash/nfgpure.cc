//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Algorithm to compute pure strategy equilibria on normal forms
//

#include "nfgpure.h"

#include "base/base.h"
#include "game/nfg.h"
#include "game/nfgiter.h"
#include "game/nfgciter.h"

gList<MixedSolution> nfgEnumPure::Solve(const NFSupport &p_support,
					gStatus &p_status)
{
  const Nfg &nfg = p_support.Game();
  gList<MixedSolution> solutions;
  NfgContIter citer(p_support);

  int ncont = 1;
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    ncont *= p_support.NumStrats(pl);
  }

  int contNumber = 1;
  do  {
    p_status.Get();
    p_status.SetProgress((double) contNumber / (double) ncont);

    bool flag = true;
    NfgIter niter(citer);
    
    for (int pl = 1; flag && pl <= nfg.NumPlayers(); pl++)  {
      gNumber current = nfg.Payoff(citer.GetOutcome(), pl);
      for (int i = 1; i <= p_support.NumStrats(pl); i++)  {
	niter.Next(pl);
	if (nfg.Payoff(niter.GetOutcome(), pl) > current)  {
	  flag = false;
	  break;
	}
      }
    }
    
    if (flag)  {
      MixedProfile<gNumber> temp(p_support.Game());
      ((gVector<gNumber> &) temp).operator=(gNumber(0));
      gArray<int> profile = citer.Get();
      for (int pl = 1; pl <= profile.Length(); pl++) {
	temp(pl, profile[pl]) = 1;
      }
      
      solutions.Append(MixedSolution(temp, algorithmNfg_ENUMPURE));
    }
    contNumber++;
  }  while ((m_stopAfter == 0 || solutions.Length() < m_stopAfter) &&
	    citer.NextContingency());
}



