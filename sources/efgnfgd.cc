//
// FILE: efgnfgd.cc -- efg<->nfg conversion routines for doubles
//
// $Id$
//


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__
#include "efgnfg.imp"

// The gratuitious casts are put in to convince g++ 2.7.2 to compile
// this function
Nfg<double> *AssociatedNfg(Efg<double> *E)
{
  if ((Lexicon<double> *) E->lexicon)
    return ((Lexicon<double> *) E->lexicon)->N;
  else
    return 0;
}

Nfg<double> *AssociatedAfg(Efg<double> *E)
{
  return E->afg;
}

TEMPLATE void ClearNodeProbs(TypedNode<double> *);

TEMPLATE void MixedToBehav(const Nfg<double> &,const MixedProfile<double> &,
			   const Efg<double> &, BehavProfile<double> &);

TEMPLATE void BehavToMixed(const Efg<double> &, const BehavProfile<double> &,
			   const Nfg<double> &, MixedProfile<double> &);

TEMPLATE void RealizationProbs(const Nfg<double> &N, const MixedProfile<double> &mp,
			       const Efg<double> &E, BehavProfile<double> &bp,
			       int pl, const gArray<int> *const actions, Node *);

TEMPLATE void BehaviorStrat(const Efg<double> &E, BehavProfile<double> &bp, int pl, Node *n);

TEMPLATE Nfg<double> *MakeReducedNfg(Efg<double> &, const EFSupport &);

TEMPLATE Nfg<double> *MakeAfg(Efg<double> &);

TEMPLATE void SetEfg(Nfg<double> *, Efg<double> *);
TEMPLATE class Lexicon<double>;

