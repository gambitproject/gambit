//
// FILE: efgrat.cc -- Instantiation of rational-precision extensive forms
//
// $Id$
//

#include "rational.h"
#include "efg.imp"

template class BehavNode<gRational>;
template class BehavProfile<gRational>;
template gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);

void PureBehavProfile<gRational>::Payoff(Node *n, gRational prob, 
					 gArray<gRational> &payoff) const
{
  if (n->GetOutcome())
    for (int pl = 1; pl <= E->NumPlayers(); pl++)
      payoff[pl] += prob * (*payoffs)(n->GetOutcome()->GetNumber(), pl);
  
  if (n->GetInfoset() && n->GetPlayer()->IsChance())
    for (int i = 1; i <= n->NumChildren(); i++)
      Payoff(n->GetChild(i),
	     prob * E->GetChanceProb(n->GetInfoset(), i).operator gRational(), payoff);
  else if (n->GetInfoset())
    Payoff(n->GetChild((*profile[n->GetPlayer()->GetNumber()])[n->GetInfoset()->GetNumber()]->GetNumber()),
	   prob, payoff);
}

void PureBehavProfile<gRational>::InfosetProbs(Node *n, gRational prob,
					       gPVector<gRational> &probs) const
{
  if (n->GetInfoset() && n->GetPlayer()->IsChance())
    for (int i = 1; i <= n->NumChildren(); i++)
      InfosetProbs(n->GetChild(i),
		   prob * E->GetChanceProb(n->GetInfoset(), i).operator gRational(), probs);
  else if (n->GetInfoset())  {
    probs(n->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber()) += prob;
    InfosetProbs(n->GetChild((*profile[n->GetPlayer()->GetNumber()])[n->GetInfoset()->GetNumber()]->GetNumber()),
		 prob, probs);
  }
}

template class PureBehavProfile<gRational>;

#include "garray.imp"

template class gArray<BehavNode<gRational> *>;

#include "glist.imp"

template class gList<BehavProfile<gRational> >;
