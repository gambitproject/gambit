//
// FILE: nfgrat.cc -- Instantiation of rational-precision normal forms
//
// $Id$
//

#include "rational.h"
#include "nfg.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gArray<double>;
class gArray<gRational>;
class NFStrategySet;
class gArray<NFStrategySet *>;

class Nfg<double>;
template <class T> class gList;
class gList<gString>;
class gList<gRational>;
template <class T> class gNode;
class gNode<gString>;
class gNode<gRational>;
class Node;
class gList<Node *>;
class gNode<Node *>;
#define TEMPLATE
#pragma option -Jgd
#endif // __GNUG__, __BORLANDC__

#include "nfg.imp"
#include "readnfg.imp"
#include "mixedsol.imp"

TEMPLATE class Nfg<gRational>;
DataType Nfg<gRational>::Type(void) const { return RATIONAL; }
gRational Nfg<gRational>::NullPayoff(void) const  { return gRational(0); }

TEMPLATE class NfgFile<gRational>;

NfgFile<gRational>::NfgFile(gInput &f, Nfg<gRational> *& N)
  : NfgFileReader(f, fooD, N), type(RATIONAL), fooD(0)
{ }


TEMPLATE class MixedProfile<gRational>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

TEMPLATE class MixedSolution<gRational>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<gRational> &);

#include "glist.imp"

TEMPLATE class gList<MixedProfile<gRational> >;
TEMPLATE class gNode<MixedProfile<gRational> >;

TEMPLATE class gList<MixedSolution<gRational> >;
TEMPLATE class gNode<MixedSolution<gRational> >;

#include "grblock.imp"

TEMPLATE class gRectBlock<gRational>;
