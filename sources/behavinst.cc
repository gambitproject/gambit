//
// FILE: behavinst.cc -- Instantiation of behavior profile template classes
//
// $Id$
//

#include "behav.imp"
#include "math/rational.h"

class ChanceInfoset;

TEMPLATE_SPECIALIZATION()
gNumber BehavProfile<gNumber>::Payoff(FullEfg::Outcome *p_outcome,
				      int pl) const
{ 
  return p_outcome->m_payoffs[pl];
}

TEMPLATE_SPECIALIZATION()
gRational BehavProfile<gRational>::Payoff(FullEfg::Outcome *p_outcome,
					  int pl) const
{ 
  return p_outcome->m_payoffs[pl];
}

TEMPLATE_SPECIALIZATION()
double BehavProfile<double>::Payoff(FullEfg::Outcome *p_outcome, int pl) const
{ 
  return p_outcome->m_doublePayoffs[pl];
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

#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<BehavProfile<double>::BehavNode *>;
template class gArray<BehavProfile<gRational>::BehavNode *>;
template class gArray<BehavProfile<gNumber>::BehavNode *>;

template class gArray<BehavProfile<double>::BehavAction *>;
template class gArray<BehavProfile<gRational>::BehavAction *>;
template class gArray<BehavProfile<gNumber>::BehavAction *>;

template class gArray<BehavProfile<double>::BehavInfoset *>;
template class gArray<BehavProfile<gRational>::BehavInfoset *>;
template class gArray<BehavProfile<gNumber>::BehavInfoset *>;

#include "base/glist.imp"

template class gList<BehavProfile<double> >;
template class gList<BehavProfile<gRational> >;
template class gList<BehavProfile<gNumber> >;

template class gList<BehavSolution>;



