//
// FILE: behavinst.cc -- Instantiation of behavior profile template classes
//
// $Id$
//

#include "behav.imp"
#include "math/rational.h"

class ChanceInfoset;

TEMPLATE_SPECIALIZATION()
gNumber BehavProfile<gNumber>::Payoff(FullEfgNamespace::Outcome *p_outcome,
				      int pl) const
{ 
  return p_outcome->m_payoffs[pl];
}

TEMPLATE_SPECIALIZATION()
gRational BehavProfile<gRational>::Payoff(FullEfgNamespace::Outcome *p_outcome,
					  int pl) const
{ 
  return p_outcome->m_payoffs[pl];
}

TEMPLATE_SPECIALIZATION()
double BehavProfile<double>::Payoff(FullEfgNamespace::Outcome *p_outcome, int pl) const
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

#include "base/glist.imp"

template class gList<BehavProfile<double> >;
template class gList<BehavProfile<gRational> >;
template class gList<BehavProfile<gNumber> >;



