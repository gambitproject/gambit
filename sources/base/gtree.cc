//
// FILE: gtree.cc -- Provide implementations for commonly-used lists
//
// $Id$
//

#include "base/base.h"
#include "rational.h"
#include "double.h"
#include "complex.h"
#include "gpoly.h"
#include "gvector.h"

#include "base/gtree.imp"
#include "base/glist.imp"

//template class gTreeNode<gPoly<double> >;
//template class gList<gTreeNode<gPoly<double> >*>;
//template class gTree<gPoly<double> >;
//template class gList<gTree<gPoly<double> > >;
//template gOutput& operator << (gOutput& output, 
//			       const gTree<gPoly<double> >&);

template class gTreeNode<gPoly<gRational> >;
template class gList<gTreeNode<gPoly<gRational> >*>;
template class gTree<gPoly<gRational> >;
template class gList<gTree<gPoly<gRational> > >;
template gOutput& operator << (gOutput& output, 
			       const gTree<gPoly<gRational> >&);

template class gTreeNode<gPoly<gDouble> >;
template class gList<gTreeNode<gPoly<gDouble> >*>;
template class gTree<gPoly<gDouble> >;
template class gList<gTree<gPoly<gDouble> > >;
template gOutput& operator << (gOutput& output, 
			       const gTree<gPoly<gDouble> >&);


