//
// FILE: ludecomp.cc -- Instantiation of LU decomposition classes
//
// $Id$
//

#include "ludecomp.imp"
#include "glist.imp"
#include "rational.h"

template class EtaMatrix< double >;
template class gList< EtaMatrix< double > >;
template class Basis< double >;
template class LUdecomp< double >;

template gOutput& operator<<( gOutput&, const EtaMatrix< double > &); 

template class EtaMatrix< gRational >;
template class gList< EtaMatrix< gRational > >;
template class Basis< gRational >;
template class LUdecomp< gRational >;

template gOutput& operator<<( gOutput&, const EtaMatrix< gRational > &); 

