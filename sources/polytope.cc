//
// FILE: polytope.cc -- Instantiation of gPolytope class
//
//  $Id$
//

#include "base/gmisc.h"
#include "polytope.imp"
#include "rational.h"

#include "glist.imp"

template class gList<index_pair*>;
//template class gNode<index_pair*>;
template class gList<gVector<int> >;
//template class gNode<gVector<int> >;
//template class gList<gVector<gRational> >;
//template class gNode<gVector<gRational> >;

template class gPolytope<int>;
template gOutput &operator<<(gOutput &f, const gPolytope<int> &y);

template class gPolytope<gRational>;
template gOutput &operator<<(gOutput &f, const gPolytope<gRational> &y);




