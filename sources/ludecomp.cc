//#
//# FILE: ludecomp.cc -- Instantiation of LU decomposition classes
//#
//# $Id$
//#

#ifdef __GNUG__
#define TEMPLATE template
#elif defined(__BORLANDC__)
template <class T> class gList;
class gList<int>;
template <class T> class gNode;
class gNode<int>;
#pragma option -Jgd
#define TEMPLATE
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__

#include "ludecomp.imp"
#include "glist.imp"
#include "rational.h"

TEMPLATE class EtaMatrix< double >;
TEMPLATE class gList< EtaMatrix< double > >;
TEMPLATE class gNode< EtaMatrix< double > >;
TEMPLATE class Basis< double >;
TEMPLATE class LUdecomp< double >;

TEMPLATE gOutput& operator<<( gOutput&, const EtaMatrix< double > &); 

TEMPLATE class EtaMatrix< gRational >;
TEMPLATE class gList< EtaMatrix< gRational > >;
TEMPLATE class gNode< EtaMatrix< gRational > >;
TEMPLATE class Basis< gRational >;
TEMPLATE class LUdecomp< gRational >;

TEMPLATE gOutput& operator<<( gOutput&, const EtaMatrix< gRational > &); 

