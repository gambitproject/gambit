//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "glist.h"

#include "efg.h"
#include "behavsol.h"


template <class T> int FindPureNash(const Efg<T> &efg,
				    gList<BehavSolution<T> > &eqs)
{
  gVector<int> isets(efg.NumPlayers());
  for (int i = 1; i <= efg.NumPlayers(); i++)
    isets[i] = efg.PlayerList()[i]->NumInfosets();

  gPVector<int> profile(isets);
  profile = 1;
  
  gVector<T> payoff(efg.NumPlayers()), deviate(efg.NumPlayers());

  while (true)   {
    // check if the current profile is a PSNE
    efg.Payoff(profile, payoff);

    bool is_psne = true;

    for (int i = 1; i <= efg.NumPlayers() && is_psne; i++)  {
      for (int j = 1; j <= isets[i] && is_psne; j++)  {
	int foo = profile(i, j);

	for (int k = 1; is_psne &&
	     k <= efg.PlayerList()[i]->InfosetList()[j]->NumActions(); k++)  {
	  profile(i, j) = k;
	  efg.Payoff(profile, deviate);
	  if (deviate[i] > payoff[i])  is_psne = false;
	}

	profile(i, j) = foo;
      }
    }
    
    if (is_psne)    {
      BehavProfile<T> tmp(efg);
      ((gVector<T> &) tmp).operator=((T) 0);

      for (int i = 1; i <= efg.NumPlayers(); i++)
	for (int j = 1; j <= isets[i]; j++)
	  tmp(i, j, profile(i, j)) = (T) 1;

      BehavSolution<T> sol(tmp, id_PURENASH);
      sol.SetIsNash(T_YES);
      eqs.Append(sol);
    }

    // go to next profile

    int pl = efg.NumPlayers();
    int iset = isets[pl];
    
    while (true)   {
      if (profile(pl, iset) < efg.PlayerList()[pl]->InfosetList()[iset]->NumActions())  {
	profile(pl, iset) += 1;
	break;
      }

      profile(pl, iset) = 1;
      iset--;
      if (iset == 0)  {
	pl--;
	if (pl == 0)   break;
	iset = isets[pl];
      }
    }

    
    if (pl == 0)   break;
  }

  return eqs.Length();
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE int FindPureNash(const Efg<double> &, gList<BehavSolution<double> > &);
TEMPLATE int FindPureNash(const Efg<gRational> &,gList<BehavSolution<gRational> > &);

