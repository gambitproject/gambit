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

template class PureBehavProfile<gRational>;

#include "efgiter.imp"

template class EfgIter<gRational>;

#include "efgciter.imp"

template class EfgContIter<gRational>;

#include "garray.imp"

template class gArray<BehavNode<gRational> *>;

#include "glist.imp"

template class gList<BehavProfile<gRational> >;
template class gNode<BehavProfile<gRational> >;
