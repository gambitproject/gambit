//
// File: quiksolv.cc  
//        -- Instantiations of classes 
// 
// @(#)quiksolv.cc	1.1 1/7/98
//

#include "quiksolv.imp"
#include "double.h"

//template class QuikSolv<int>;
//template gOutput &operator<<(gOutput &f, const QuikSolv<int> &y);

template class QuikSolv<gRational>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gRational> &y);

template class QuikSolv<double>;
template gOutput &operator<<(gOutput &f, const QuikSolv<double> &y);

template class QuikSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gDouble> &y);

#include "glist.imp"

//Experimental, eventually should preceed QuikSolv
template class TreeOfPartials<gDouble>;
template gOutput &operator<<(gOutput &f, const TreeOfPartials<gDouble> &y);
template class gList<TreeOfPartials<gDouble> >;
template class gNode<TreeOfPartials<gDouble> >;
template class ListOfPartialTrees<gDouble>;
