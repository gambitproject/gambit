//
// File: ideal.cc  
//        -- Instantiations of classes gBasis and gIdeal
// @(#)gpolylst.cc	1.1 1/7/98 
//

#include "gpolylst.imp"
#include "double.h"
#include "glist.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

//TEMPLATE class gPolyList<int>;
//TEMPLATE gOutput &operator<<(gOutput &f, const gPolyList<int> &y);

TEMPLATE class gPolyList<gRational>;
TEMPLATE gOutput &operator<<(gOutput &f, const gPolyList<gRational> &y);

TEMPLATE class gPolyList<double>;
TEMPLATE gOutput &operator<<(gOutput &f, const gPolyList<double> &y);

TEMPLATE class gPolyList<gDouble>;
TEMPLATE gOutput &operator<<(gOutput &f, const gPolyList<gDouble> &y);

TEMPLATE class gList<index_pair>;
TEMPLATE class gNode<index_pair>;

/*
#include "objcount.imp"

template class Counted<gPolyList<gDouble> >;
int Counted<gPolyList<gDouble> >::numObjects = 0;

template class Counted<gPolyList<double> >;
int Counted<gPolyList<double> >::numObjects = 0;

template class Counted<gPolyList<gRational> >;
int Counted<gPolyList<gRational> >::numObjects = 0;
*/

