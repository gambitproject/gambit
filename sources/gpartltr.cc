//
// File: gpartltr.cc  
//        -- Instantiations of classes 
// 
// @(#)gpartltr.cc	1.1 1/7/98
//

#include "gpartltr.imp"
//#include "double.h"
#include "glist.imp"

template class TreeOfPartials<gDouble>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gDouble> &y);
template class gList<TreeOfPartials<gDouble> >;
template class gList<gList<TreeOfPartials<gDouble> > >;
template class gList<gList<gList<TreeOfPartials<gDouble> > > >;
template class ListOfPartialTrees<gDouble>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<gDouble> &y);

template class TreeOfPartials<gRational>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gRational> &y);
template class gList<TreeOfPartials<gRational> >;
template class gList<gList<TreeOfPartials<gRational> > >;
template class gList<gList<gList<TreeOfPartials<gRational> > > >;
template class ListOfPartialTrees<gRational>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<gRational> &y);


template class TreeOfPartials<double>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<double> &y);
template class gList<TreeOfPartials<double> >;
template class gList<gList<TreeOfPartials<double> > >;
template class gList<gList<gList<TreeOfPartials<double> > > >;
template class ListOfPartialTrees<double>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<double> &y);

