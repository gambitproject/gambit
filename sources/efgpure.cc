//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "glist.h"

#include "efg.h"
#include "efgiter.h"
#include "efgciter.h"
#include "behavsol.h"

#include "efgpure.h"

template <class T> int FindPureNash(const Efg<T> &efg,
				    gList<BehavSolution<T> > &eqs)
{
  int index;
  EFSupport S(efg);
  EfgContIter<T> citer(S);

  do  {
    int flag = 1;
    EfgIter<T> eiter(citer);

    for (int pl = 1; flag && pl <= efg.NumPlayers(); pl++)  {
      T current = citer.Payoff(pl);
      for (int iset = 1; flag && iset <= efg.PlayerList()[pl]->NumInfosets();
	   iset++)  {
	Infoset *s = efg.PlayerList()[pl]->InfosetList()[iset];
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
      BehavProfile<T> temp(efg);
      // zero out all the entries, since any equlibria are pure
      ((gVector<T> &) temp).operator=((T) 0);
      gPVector<int> profile(citer.GetEfgNumbering());
      for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
	for (int iset = 1; iset <= efg.PlayerList()[pl]->NumInfosets();
	     iset++)
	  temp(pl, iset, profile(pl, iset)) = (T) 1;
      }

      index = eqs.Append(BehavSolution<T>(temp, id_PURENASH));
      eqs[index].SetIsNash(T_YES);
    }
  }  while (citer.NextContingency());

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



//-----------------------------------
// Interfacing to solve-by-subgame
//-----------------------------------

template <class T>
int EfgPSNEBySubgame<T>::SolveSubgame(const Efg<T> &E,
				      gList<BehavSolution<T> > &solns)
{
  FindPureNash(E, solns);

  return 0;
}

template <class T>
EfgPSNEBySubgame<T>::EfgPSNEBySubgame(const Efg<T> &E, int max)
  : SubgameSolver<T>(E, max)
{ }

template <class T> EfgPSNEBySubgame<T>::~EfgPSNEBySubgame()   { }
  


#ifdef __GNUG__
template class EfgPSNEBySubgame<double>;
template class EfgPSNEBySubgame<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class EfgPSNEBySubgame<double>;
class EfgPSNEBySubgame<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__
