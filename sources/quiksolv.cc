//
// File: quiksolv.cc  
//        -- Instantiations of classes 
// 
// @(#)quiksolv.cc	1.1 1/7/98
//

#include "quiksolv.imp"
#include "double.h"

template class QuikSolv<gRational>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gRational> &y);

template class QuikSolv<double>;
template gOutput &operator<<(gOutput &f, const QuikSolv<double> &y);

template class QuikSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gDouble> &y);

#include "glist.imp"

/*
// Eventually these classes should be in separate files
template class TreeOfPartials<gDouble>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gDouble> &y);
template class gList<TreeOfPartials<gDouble> >;
template class ListOfPartialTrees<gDouble>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<gDouble> &y);

template class TreeOfPartials<gRational>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gRational> &y);
template class gList<TreeOfPartials<gRational> >;
template class ListOfPartialTrees<gRational>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<gRational> &y);


template class TreeOfPartials<double>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<double> &y);
template class gList<TreeOfPartials<double> >;
template class ListOfPartialTrees<double>;
template gOutput &operator<<(gOutput &f, 
			     const ListOfPartialTrees<double> &y);

*/
