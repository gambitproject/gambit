//
// FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
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

template <class T> class gList;
class gList<gString>;
class gList<gRational>;
template <class T> class gNode;
class gNode<gString>;
class gNode<gRational>;

class Nfg<gRational>;
template <class T> class NfgIter;
class NfgIter<gRational>;
template <class T> class NfgContIter;
class NfgContIter<gRational>;


#define TEMPLATE
#pragma option -Jgd
#endif // __GNUG__, __BORLANDC__

#include "nfg.imp"
#include "nfgiter.imp"
#include "nfgciter.imp"
#include "readnfg.imp"
#include "mixedsol.imp"

TEMPLATE class Nfg<double>;
DataType Nfg<double>::Type(void) const { return DOUBLE; }

TEMPLATE class NfgFile<double>;
TEMPLATE int ReadNfgFile(gInput &, Nfg<double> *&);

TEMPLATE class NfgContIter<double>;
TEMPLATE class NfgIter<double>;
TEMPLATE class NFOutcome<double>;

TEMPLATE class MixedProfile<double>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<double> &);

TEMPLATE class MixedSolution<double>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<double> &);

#include "garray.imp"

TEMPLATE class gArray<NFOutcome<double> *>;

#include "glist.imp"

TEMPLATE class gList<MixedProfile<double> >;
TEMPLATE class gNode<MixedProfile<double> >;

TEMPLATE class gList<MixedSolution<double> >;
TEMPLATE class gNode<MixedSolution<double> >;

TEMPLATE class gList<NFOutcome<double> *>;
TEMPLATE class gNode<NFOutcome<double> *>;

