//
// FILE: efgnfgd.cc -- efg<->nfg conversion routines for doubles
//
// $Id$
//


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

template void ClearNodeProbs(BehavNode<double> *);

template void MixedToBehav(const Nfg<double> &,const MixedProfile<double> &,
			   const Efg<double> &, BehavProfile<double> &);

template void BehavToMixed(const Efg<double> &, const BehavProfile<double> &,
			   const Nfg<double> &, MixedProfile<double> &);

template void RealizationProbs(const Nfg<double> &N, const MixedProfile<double> &mp,
			       const Efg<double> &E, BehavProfile<double> &bp,
			       int pl, const gArray<int> *const actions, BehavNode<double> *);

template void BehaviorStrat(const Efg<double> &E, BehavProfile<double> &bp, int pl, BehavNode<double> *);

template Nfg<double> *MakeReducedNfg(Efg<double> &, const EFSupport &);

template Nfg<double> *MakeAfg(Efg<double> &);

template void SetEfg(Nfg<double> *, Efg<double> *);
template class Lexicon<double>;

