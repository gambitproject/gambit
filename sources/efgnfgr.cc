//
// FILE: efgnfgr.cc -- efg<->nfg conversion routines for rationals
//
// $Id$
//


#include "efgnfg.imp"

// The gratuitious casts are put in to convince g++ 2.7.2 to compile
// this function
Nfg<gRational> *AssociatedNfg(Efg<gRational> *E)
{
  if ((Lexicon<gRational> *) E->lexicon)
    return ((Lexicon<gRational> *) E->lexicon)->N;
  else
    return 0;
}


Nfg<gRational> *AssociatedAfg(Efg<gRational> *E)
{
  return E->afg;
}

template void ClearNodeProbs(BehavNode<gRational> *);

template void MixedToBehav(const Nfg<gRational> &, const MixedProfile<gRational> &,
			   const Efg<gRational> &, BehavProfile<gRational> &);

template void BehavToMixed(const Efg<gRational> &, const BehavProfile<gRational> &,
			   const Nfg<gRational> &, MixedProfile<gRational> &);

template void RealizationProbs(const Nfg<gRational> &N, const MixedProfile<gRational> &mp,
			       const Efg<gRational> &E, BehavProfile<gRational> &bp,
			       int pl, const gArray<int> *const actions, BehavNode<gRational> *);

template void BehaviorStrat(const Efg<gRational> &E, BehavProfile<gRational> &bp, int pl, BehavNode<gRational> *n);

template Nfg<gRational> *MakeReducedNfg(Efg<gRational> &, const EFSupport &);

template Nfg<gRational> *MakeAfg(Efg<gRational> &);

template void SetEfg(Nfg<gRational> *, Efg<gRational> *);
template class Lexicon<gRational>;


