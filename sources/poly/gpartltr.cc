//
// FILE: gpartltr.cc  
//        -- Instantiations of classes 
// 
// $Id$
//

#include "base/glist.imp"
#include "gpartltr.imp"

template class TreeOfPartials<gDouble>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gDouble> &y);
template class gList<TreeOfPartials<gDouble> >;
template gOutput &operator<<(gOutput &f, 
			     const gList<TreeOfPartials<gDouble> > &y);
template class gList<gList<TreeOfPartials<gDouble> > >;
template gOutput &operator<<(gOutput &f, 
			     const gList<gList<TreeOfPartials<gDouble> > > &y);
template class gList<gList<gList<TreeOfPartials<gDouble> > > >;
template class ListOfPartialTrees<gDouble>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<gDouble> &y);

template class TreeOfPartials<gRational>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gRational> &y);
template class gList<TreeOfPartials<gRational> >;
template gOutput &operator<<(gOutput &f, 
			     const gList<TreeOfPartials<gRational> > &y);
template class gList<gList<TreeOfPartials<gRational> > >;
template gOutput &operator<<(gOutput &f, 
			   const gList<gList<TreeOfPartials<gRational> > > &y);
template class gList<gList<gList<TreeOfPartials<gRational> > > >;
template class ListOfPartialTrees<gRational>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<gRational> &y);


//template class TreeOfPartials<double>;
//template gOutput &operator<<(gOutput &f, const TreeOfPartials<double> &y);
//template class gList<TreeOfPartials<double> >;
//template gOutput &operator<<(gOutput &f, 
//			     const gList<TreeOfPartials<double> > &y);
//template class gList<gList<TreeOfPartials<double> > >;
//template gOutput &operator<<(gOutput &f, 
//			     const gList<gList<TreeOfPartials<double> > > &y);
//template class gList<gList<gList<TreeOfPartials<double> > > >;
//template class ListOfPartialTrees<double>;
//template gOutput &operator<<(gOutput &f, 
//			     const ListOfPartialTrees<double> &y);

#include "base/gtree.imp"

template class gTreeNode<gPoly<gRational> >;
template class gList<gTreeNode<gPoly<gRational> >*>;
template class gTree<gPoly<gRational> >;
template class gList<gTree<gPoly<gRational> > >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, 
			       const gTree<gPoly<gRational> >&);
#endif  // __BCC55__

template class gTreeNode<gPoly<gDouble> >;
template class gList<gTreeNode<gPoly<gDouble> >*>;
template class gTree<gPoly<gDouble> >;
template class gList<gTree<gPoly<gDouble> > >;
#ifndef __BCC55__
template gOutput& operator << (gOutput& output, 
			       const gTree<gPoly<gDouble> >&);
#endif  // __BCC55__
