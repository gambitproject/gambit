//
// FILE: efgdbl.cc -- Instantiation of double-precision extensive forms
//
// $Id$
//

#include "rational.h"
#include "glist.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__

template <class T> class TypedNode;
class TypedNode<gRational>;
template <class T> class BehavProfile;
class BehavProfile<gRational>;
template <class T> class Efg;
class Efg<gRational>;
template <class T> class OutcomeVector;
class OutcomeVector<gRational>;
template <class T> class ChanceInfoset;
class ChanceInfoset<gRational>;
class Node;
class gString;
class gList<gString>;
class gList<gRational>;
class gList<Node *>;

template <class T> class BehavSolution;
gOutput &operator<<(gOutput &, const BehavSolution<double> &);

#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "efg.imp"
TEMPLATE class Efg<double>;
DataType Efg<double>::Type(void) const    { return DOUBLE; }

TEMPLATE class TypedNode<double>;
TEMPLATE class ChanceInfoset<double>;
TEMPLATE class OutcomeVector<double>;

TEMPLATE class BehavProfile<double>;
TEMPLATE gOutput &operator<<(gOutput &, const BehavProfile<double> &);

#include "behavsol.imp"

unsigned long BehavSolution<double>::MaxId=1;
TEMPLATE class BehavSolution<double>;
TEMPLATE gOutput &operator<<(gOutput &, const BehavSolution<double> &);


#include "efgiter.imp"

TEMPLATE class EfgIter<double>;

#include "efgciter.imp"

TEMPLATE class EfgContIter<double>;

#include "readefg.imp"

TEMPLATE class EfgFile<double>;
TEMPLATE int ReadEfgFile(gInput &, Efg<double> *&);

#include "glist.imp"

TEMPLATE class gList<BehavProfile<double> >;
TEMPLATE class gNode<BehavProfile<double> >;

TEMPLATE class gList<BehavSolution<double> >;
TEMPLATE class gNode<BehavSolution<double> >;




