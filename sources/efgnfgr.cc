//
// FILE: efgnfgr.cc -- efg<->nfg conversion routines for rationals
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

TEMPLATE void ClearNodeProbs(TypedNode<gRational> *);

TEMPLATE void MixedToBehav(const Nfg<gRational> &, const MixedProfile<gRational> &,
			   const Efg<gRational> &, BehavProfile<gRational> &);

TEMPLATE void BehavToMixed(const Efg<gRational> &, const BehavProfile<gRational> &,
			   const Nfg<gRational> &, MixedProfile<gRational> &);

TEMPLATE void RealizationProbs(const Nfg<gRational> &N, const MixedProfile<gRational> &mp,
			       const Efg<gRational> &E, BehavProfile<gRational> &bp,
			       int pl, const gArray<int> *const actions, Node *);

TEMPLATE void BehaviorStrat(const Efg<gRational> &E, BehavProfile<gRational> &bp, int pl, Node *n);

TEMPLATE Nfg<gRational> *MakeReducedNfg(Efg<gRational> &);

TEMPLATE Nfg<gRational> *MakeAfg(Efg<gRational> &);

