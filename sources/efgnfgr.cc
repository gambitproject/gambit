//
// FILE: efgnfgr.cc -- efg<->nfg conversion routines for rationals
//
// $Id$
//


#include "efgnfg.imp"

// The gratuitious casts are put in to convince g++ 2.7.2 to compile
// this function
Nfg *AssociatedNfg(const Efg *E)
{
  if (E->lexicon)
    return E->lexicon->N;
  else
    return 0;
}


Nfg *AssociatedAfg(const Efg *E)
{
  return E->afg;
}

template void ClearNodeProbs(BehavNode<gRational> *);

template void ClearNodeProbs(BehavNode<gNumber> *);

template void MixedToBehav(const Nfg &, const MixedProfile<gRational> &,
			   const Efg &, BehavProfile<gRational> &);

template void MixedToBehav(const Nfg &, const MixedProfile<gNumber> &,
			   const Efg &, BehavProfile<gNumber> &);

template void BehavToMixed(const Efg &, const BehavProfile<gRational> &,
			   const Nfg &, MixedProfile<gRational> &);

template void BehavToMixed(const Efg &, const BehavProfile<gNumber> &,
			   const Nfg &, MixedProfile<gNumber> &);

template void RealizationProbs(const MixedProfile<gRational> &mp,
			       const Efg &E, BehavProfile<gRational> &bp,
			       int pl, const gArray<int> *const actions, BehavNode<gRational> *);

template void RealizationProbs(const MixedProfile<gNumber> &mp,
			       const Efg &E, BehavProfile<gNumber> &bp,
			       int pl, const gArray<int> *const actions, BehavNode<gNumber> *);

template void BehaviorStrat(const Efg &E, BehavProfile<gRational> &bp, int pl, BehavNode<gRational> *n);

template void BehaviorStrat(const Efg &E, BehavProfile<gNumber> &bp, int pl, BehavNode<gNumber> *n);

