//#
//# FILE: nfgrat.cc -- Instantiation of rational-precision normal forms
//#
//# $Id$
//#

#include "rational.h"
#include "nfg.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__

class Nfg<double>;

#define TEMPLATE
#pragma option -Jgd
#endif // __GNUG__, __BORLANDC__

#include "nfg.imp"
#include "nfgiter.imp"
#include "contiter.imp"
#include "readnfg.imp"

TEMPLATE class Nfg<gRational>;
DataType Nfg<gRational>::Type(void) const { return RATIONAL; }

TEMPLATE class NfgFile<gRational>;
TEMPLATE int ReadNfgFile(gInput &, Nfg<gRational> *&);

TEMPLATE class NfgIter<gRational>;
TEMPLATE class ContIter<gRational>;

TEMPLATE class MixedProfile<gRational>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

#include "garray.imp"

TEMPLATE class gArray<gRational *>;

#include "glist.imp"

TEMPLATE class gList<MixedProfile<gRational> >;
TEMPLATE class gNode<MixedProfile<gRational> >;
