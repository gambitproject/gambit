//
// FILE: glist.cc -- Provide implementations for commonly-used lists
//
// $Id$
//

#include "gmisc.h"
#include "glist.imp"
#include "gstring.h"
#include "rational.h"
#include "gnumber.h"

#include "garray.h"
#include "gblock.h"


template class gList<gRational>;
template class gNode<gRational>;

template class gList<gNumber>;
template class gNode<gNumber>;

template class gList<int>;
template class gNode<int>;

template class gList<long>;
template class gNode<long>;

template class gList<double>;
template class gNode<double>;

template class gList<gString>;
template class gNode<gString>;

template class gList<gBlock<int> >;
template class gNode<gBlock<int> >;

template class gList<gArray<int> >;
template class gNode<gArray<int> >;



