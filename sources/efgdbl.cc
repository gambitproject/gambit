//
// FILE: efgdbl.cc -- Instantiation of double-precision extensive forms
//
// $Id$
//

#include "rational.h"


#include "efg.imp"
template class Efg<double>;
DataType Efg<double>::Type(void) const    { return gDOUBLE; }

template class ChanceInfoset<double>;

template class BehavNode<double>;
template class BehavProfile<double>;
template gOutput &operator<<(gOutput &, const BehavProfile<double> &);

#include "behavsol.imp"

template class BehavSolution<double>;
template gOutput &operator<<(gOutput &, const BehavSolution<double> &);


#include "efgiter.imp"

template class EfgIter<double>;

#include "efgciter.imp"

template class EfgContIter<double>;

#include "readefg.imp"

template class EfgFile<double>;
template int ReadEfgFile(gInput &, Efg<double> *&);

#include "garray.imp"

template class gArray<BehavNode<double> *>;

#include "glist.imp"

template class gList<BehavProfile<double> >;
template class gNode<BehavProfile<double> >;

template class gList<BehavSolution<double> >;
template class gNode<BehavSolution<double> >;




