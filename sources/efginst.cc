//
// FILE: efginst.cc -- Instantiation of extensive form template classes
//
// 
//

#include "behav.imp"
#include "rational.h"

template class BehavNode<double>;
template class BehavProfile<double>;
template class BehavAssessment<double>;
template gOutput &operator<<(gOutput &, const BehavProfile<double> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<double> &);

template class BehavNode<gRational>;
template class BehavProfile<gRational>;
template class BehavAssessment<gRational>;
template gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<gRational> &);

template class BehavNode<gNumber>;
template class BehavProfile<gNumber>;
template class BehavAssessment<gNumber>;
template gOutput &operator<<(gOutput &, const BehavProfile<gNumber> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<gNumber> &);


template class PureBehavProfile<double>;

void PureBehavProfile<gRational>::Payoff(const Node *n, const gRational prob, 
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
template class PureBehavProfile<gNumber>;

#include "garray.imp"
#include "gblock.imp"

template class gArray<BehavNode<double> *>;
template class gArray<BehavNode<gRational> *>;
template class gArray<BehavNode<gNumber> *>;

#include "glist.imp"

template class gList<BehavProfile<double> >;
template class gList<BehavProfile<gRational> >;
template class gList<BehavProfile<gNumber> >;


template class gArray<EFPlayer *>;
template class gBlock<EFPlayer *>;

template class gArray<Infoset *>;
template class gBlock<Infoset *>;

template class gArray<Node *>;
template class gBlock<Node *>;

template class gArray<Action const *>;
template class gBlock<Action const *>;
template class gArray<Action *>;
template class gBlock<Action *>;

template class gArray<gArray<Action const *> *>;

template class gArray<gArray<Action *> *>;

template class gArray<EFOutcome *>;
template class gBlock<EFOutcome *>;

template class gArray<gBlock<bool> >;
template class gBlock<gBlock<bool> >;

class EFActionSet;
class EFActionArray;

template bool operator==(const gArray<Action *> &, const gArray<Action *> &);
template class gArray<EFActionSet *>;
template class gArray<EFActionArray *>;

template class gList<Node *>;
template class gList<const Node *>;
template gOutput &operator<<(gOutput &, const gList<const Node *> &);

template class gList<Action *>;
template class gList<const Action *>;

template class gList<Infoset *>;

template class gList<BehavSolution>;



