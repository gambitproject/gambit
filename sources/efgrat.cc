//
// FILE: efgrat.cc -- Instantiation of rational-precision extensive forms
//
// $Id$
//

#include "rational.h"
#include "efg.imp"

template class Efg<gRational>;
DataType Efg<gRational>::Type(void) const   { return gRATIONAL; }

template class TypedNode<gRational>;
template class ChanceInfoset<gRational>;
template class BehavProfile<gRational>;
template gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);

#include "behavsol.imp"

template class BehavSolution<gRational>;

template gOutput &operator<<(gOutput &, const BehavSolution<gRational> &);


#include "efgiter.imp"

template class EfgIter<gRational>;

#include "efgciter.imp"

template class EfgContIter<gRational>;

#include "readefg.imp"

template class EfgFile<gRational>;
template int ReadEfgFile(gInput &, Efg<gRational> *&);

#include "glist.imp"

template class gList<BehavProfile<gRational> >;
template class gNode<BehavProfile<gRational> >;

template class gList<BehavSolution<gRational> >;
template class gNode<BehavSolution<gRational> >;


