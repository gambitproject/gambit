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

TEMPLATE void ClearNodeProbs(TypedNode<double> *);

TEMPLATE void MixedToBehav(const Nfg<double> &,const MixedProfile<double> &,
			   const Efg<double> &, BehavProfile<double> &);

TEMPLATE void RealizationProbs(const Nfg<double> &N, const MixedProfile<double> &mp,
			       const Efg<double> &E, BehavProfile<double> &bp,
			       int pl, const gArray<int> *const actions, Node *);

TEMPLATE void BehaviorStrat(const Efg<double> &E, BehavProfile<double> &bp, int pl, Node *n);

TEMPLATE Nfg<double> *MakeReducedNfg(Efg<double> &);

TEMPLATE Nfg<double> *MakeAfg(Efg<double> &);

