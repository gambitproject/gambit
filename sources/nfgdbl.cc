//#
//# FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
//#
//# $Id$
//#

#include "rational.h"
#include "nfg.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__

class Nfg<gRational>;

#define TEMPLATE
#pragma option -Jgd
#endif // __GNUG__, __BORLANDC__

#include "nfg.imp"
#include "nfgiter.imp"
#include "contiter.imp"
#include "readnfg.imp"

TEMPLATE class Nfg<double>;
DataType Nfg<double>::Type(void) const { return DOUBLE; }

TEMPLATE class NfgFile<double>;
TEMPLATE int ReadNfgFile(gInput &, Nfg<double> *&);

TEMPLATE class ContIter<double>;
TEMPLATE class NfgIter<double>;

TEMPLATE class MixedProfile<double>;
TEMPLATE gOutput &operator<<(gOutput &, const MixedProfile<double> &);

#include "garray.imp"

TEMPLATE class gArray<double *>;

#include "glist.imp"

TEMPLATE class gList<MixedProfile<double> >;
TEMPLATE class gNode<MixedProfile<double> >;



