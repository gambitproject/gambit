//
// FILE: efgnfgd.cc -- efg<->nfg conversion routines for doubles
//
// $Id$
//


#include "efgnfg.imp"


template void ClearNodeProbs(BehavNode<double> *);

template void MixedToBehav(const Nfg &,const MixedProfile<double> &,
			   const Efg &, BehavProfile<double> &);

template void BehavToMixed(const Efg &, const BehavProfile<double> &,
			   const Nfg &, MixedProfile<double> &);

template void RealizationProbs(const Nfg &N, const MixedProfile<double> &mp,
			       const Efg &E, BehavProfile<double> &bp,
			       int pl, const gArray<int> *const actions, BehavNode<double> *);

template void BehaviorStrat(const Efg &E, BehavProfile<double> &bp, int pl, BehavNode<double> *);


