//#
//# FILE: purenash.cc -- Find all pure strategy Nash equilibria
//#
//# $Id$
//#

#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "contiter.h"
#include "rational.h"
#include "glist.h"
#include "gtuple.h"

template <class T> int FindPureNash(const NormalForm<T> &N,
				    gList<gTuple<int> > &eqs)
{
  ContIter<T> citer((NormalForm<T> &) N);

  do  {
    int flag = 1;

    for (int pl = 1; flag && pl <= N.NumPlayers(); pl++)  {
      T current = citer.Payoff(pl);
      NormalIter<T> niter(citer);
      for (int i = 2; i <= N.NumStrats(pl); i++)  {
	niter.Next(pl);
	T pay = niter.Payoff(pl);
	if (pay > current)  {
	  flag = 0;
	  break;
	}
      }
    }

    if (flag) 
      eqs.Append(citer.Get());
  }  while (citer.NextContingency());

  return eqs.Length();
}

template int FindPureNash(const NormalForm<double> &, gList<gTuple<int> > &);
template int FindPureNash(const NormalForm<gRational> &,gList<gTuple<int> > &);

