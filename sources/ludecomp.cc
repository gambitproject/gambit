//
// FILE: ludecomp.cc -- Instantiation of LU decomposition classes
//
// $Id$
//

#include "ludecomp.imp"
#include "base/glist.imp"
#include "rational.h"

template class EtaMatrix< double >;
template class gList< EtaMatrix< double > >;
template class LUdecomp< double >;

template gOutput& operator<<( gOutput&, const EtaMatrix< double > &); 

template class EtaMatrix< gRational >;
template class gList< EtaMatrix< gRational > >;
template class LUdecomp< gRational >;

template gOutput& operator<<( gOutput&, const EtaMatrix< gRational > &); 

