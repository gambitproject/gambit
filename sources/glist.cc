//
// FILE: glist.cc -- Provide implementations for commonly-used lists
//
// $Id$
//

#include "gmisc.h"
#include "glist.imp"
#include "gtext.h"
#include "rational.h"
#include "gnumber.h"

#include "garray.h"
#include "gblock.h"


template class gList<gRational>;
template class gList<gNumber>;
template class gList<int>;
template class gList<long>;
template class gList<double>;
template class gList<gText>;

template class gList<gBlock<int> >;
template class gList<gArray<int> >;


