//
// FILE: efgdbl.cc -- Instantiation of double-precision extensive forms
//
// $Id$
//

#include "behav.imp"

template class BehavNode<double>;
template class BehavProfile<double>;
template class BehavAssessment<double>;
template gOutput &operator<<(gOutput &, const BehavProfile<double> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<double> &);

template class PureBehavProfile<double>;

#include "garray.imp"

template class gArray<BehavNode<double> *>;

#include "glist.imp"

template class gList<BehavProfile<double> >;




