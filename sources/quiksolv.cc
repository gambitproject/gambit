//
// File: quiksolv.cc  
//        -- Instantiations of classes 
// 
// @(#)quiksolv.cc	1.1 1/7/98
//

#include "quiksolv.imp"
#include "double.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

//TEMPLATE class QuikSolv<int>;
//TEMPLATE gOutput &operator<<(gOutput &f, const QuikSolv<int> &y);

TEMPLATE class QuikSolv<gRational>;
TEMPLATE gOutput &operator<<(gOutput &f, const QuikSolv<gRational> &y);

TEMPLATE class QuikSolv<double>;
TEMPLATE gOutput &operator<<(gOutput &f, const QuikSolv<double> &y);

TEMPLATE class QuikSolv<gDouble>;
TEMPLATE gOutput &operator<<(gOutput &f, const QuikSolv<gDouble> &y);

#include "glist.imp"

//Experimental, eventually should preceed QuikSolv
TEMPLATE class TreeOfPartials<gDouble>;
TEMPLATE gOutput &operator<<(gOutput &f, const TreeOfPartials<gDouble> &y);
TEMPLATE class gList<TreeOfPartials<gDouble> >;
TEMPLATE class gNode<TreeOfPartials<gDouble> >;
TEMPLATE class ListOfPartialTrees<gDouble>;
