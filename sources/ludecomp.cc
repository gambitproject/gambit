//
// FILE: ludecomp.cc -- Instantiation of LU decomposition classes
//
// $Id$
//

#include "ludecomp.imp"
#include "base/glist.imp"
#include "math/rational.h"

template class EtaMatrix< double >;
template class gList< EtaMatrix< double > >;
template class LUdecomp< double >;

#ifndef __BCC55__
template gOutput& operator<<( gOutput&, const EtaMatrix< double > &); 
#endif  // __BCC55__

template class EtaMatrix< gRational >;
template class gList< EtaMatrix< gRational > >;
template class LUdecomp< gRational >;

#ifndef __BCC55__
template gOutput& operator<<( gOutput&, const EtaMatrix< gRational > &); 
#endif  // __BCC55__
