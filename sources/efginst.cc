//
// FILE: efginst.cc -- Instantiation of extensive form template classes
//
// $Id$
//

#include "behav.imp"
#include "rational.h"

class ChanceInfoset;

const gNumber BehavProfile<gNumber>::Payoff(const EFOutcome *o, int pl) const
{ 
  return o->payoffs[pl];
}


const gRational BehavProfile<gRational>::Payoff(const EFOutcome *o, int pl) const
{ 
  //  gout << "\nin BehavProfile<gRational>::Payoff()";
  return o->payoffs[pl];
}

const double BehavProfile<double>::Payoff(const EFOutcome *o, int pl) const
{ 
  return o->double_payoffs[pl];
}

template class BehavProfile<double>;
template class BehavAssessment<double>;
template gOutput &operator<<(gOutput &, const BehavProfile<double> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<double> &);

template class BehavProfile<gRational>;
template class BehavAssessment<gRational>;
template gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<gRational> &);

template class BehavProfile<gNumber>;
template class BehavAssessment<gNumber>;
template gOutput &operator<<(gOutput &, const BehavProfile<gNumber> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<gNumber> &);

template class PureBehavProfile<gNumber>;

#include "garray.imp"
#include "gblock.imp"

template class gArray<BehavProfile<double>::BehavNode *>;
template class gArray<BehavProfile<gRational>::BehavNode *>;
template class gArray<BehavProfile<gNumber>::BehavNode *>;

template class gArray<BehavProfile<double>::BehavAction *>;
template class gArray<BehavProfile<gRational>::BehavAction *>;
template class gArray<BehavProfile<gNumber>::BehavAction *>;

template class gArray<BehavProfile<double>::BehavInfoset *>;
template class gArray<BehavProfile<gRational>::BehavInfoset *>;
template class gArray<BehavProfile<gNumber>::BehavInfoset *>;

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
template class gList<const Infoset *>;

template class gList<BehavSolution>;



