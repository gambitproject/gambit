//
// FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
//
// $Id$
//

#include "garray.h"
#include "glist.h"
#include "rational.h"
#include "gstring.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gArray<double>;
class gArray<gRational>;

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

TEMPLATE class Nfg<double>;
DataType Nfg<double>::Type(void) const { return DOUBLE; }
double Nfg<double>::NullPayoff(void) const  { return 0.0; }

TEMPLATE class NfgFile<double>;

NfgFile<double>::NfgFile(gInput &f, Nfg<double> *& N)
  : NfgFileReader(f, N, fooR), type(DOUBLE), fooR(0)
{ }

TEMPLATE class MixedProfile<double>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<double> &);

TEMPLATE class MixedSolution<double>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<double> &);

#include "glist.imp"

TEMPLATE class gList<MixedProfile<double> >;
TEMPLATE class gNode<MixedProfile<double> >;

TEMPLATE class gList<MixedSolution<double> >;
TEMPLATE class gNode<MixedSolution<double> >;

#include "grblock.imp"

TEMPLATE class gRectBlock<double>;

