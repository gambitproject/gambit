//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "glist.h"

#include "efg.h"
#include "behav.h"
#include "nfg.h"
#include "mixed.h"

template <class T> int FindPureNash(const Nfg<T> &N,
				    gList<MixedProfile<T> > &eqs);

template <class T>
void MixedToBehav(const Nfg<T> &N, const MixedProfile<T> &mp,
		  const Efg<T> &E, BehavProfile<T> &bp);

template <class T> Nfg<T> *MakeAfg(Efg<T> &E);


template <class T> int FindPureNash(const Efg<T> &efg,
				    gList<BehavProfile<T> > &eqs)
{
  Nfg<T> *N = MakeAfg((Efg<T> &) efg);

  if (!N)  return 0;

  gList<MixedProfile<T> > solns;
  FindPureNash(*N, solns);
  
  for (int i = 1; i <= solns.Length(); i++)  {
    BehavProfile<T> bp(efg);
    MixedToBehav(*N, solns[i], efg, bp);
    eqs.Append(bp);
  }

  return eqs.Length();
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE int FindPureNash(const Efg<double> &, gList<BehavProfile<double> > &);
TEMPLATE int FindPureNash(const Efg<gRational> &,gList<BehavProfile<gRational> > &);

